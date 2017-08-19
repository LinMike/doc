

#include "contour_fifo.h"
#include <stdlib.h>
#include "../common.h"
#include <string.h>
#include <assert.h>

static fifo_handle_t g_contour_fifo_handle = NULL;

enum OverflowStrategy {
    OVERFLOW_STRATEGY_OVERWRITE,
    OVERFLOW_STRATEGY_DISCARD
};

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int size;
	int windex;
	int rindex;
	int capability;
	pthread_mutex_t mutex;
	frame_contours_t* arr;
    enum OverflowStrategy strategy;
} contours_fifo_t;

fifo_handle_t global_fifo_get()
{
    return g_contour_fifo_handle;
}

int global_fifo_open()
{
    g_contour_fifo_handle = cfifo_create(10);
    return g_contour_fifo_handle ? 0 : -1;
}

void global_fifo_close()
{
    cfifo_free(g_contour_fifo_handle);
    g_contour_fifo_handle = NULL;
}

fifo_handle_t cfifo_create(int capacity)
{
	contours_fifo_t* handle = (contours_fifo_t*)malloc(sizeof(contours_fifo_t));
	if (handle == NULL) {
        log_error("failed to malloc the centroid_fifo handle!\n");
		return NULL;
	}
	memset(handle, 0, sizeof(contours_fifo_t));
	handle->arr = (frame_contours_t*)malloc(sizeof(frame_contours_t) * capacity);
	if (handle->arr == NULL) {
		log_error("failed to malloc the buffer for centroid\n");
		free(handle);
		return NULL;
	}
	memset(handle->arr, 0, sizeof(frame_contours_t) * capacity);
	handle->capability = capacity;
	handle->size = 0;
    handle->strategy = OVERFLOW_STRATEGY_OVERWRITE;
	pthread_mutex_init(&handle->mutex, NULL);
	return (fifo_handle_t)handle;
}

frame_contours_t* cfifo_next(fifo_handle_t handle)
{
    frame_contours_t* node = NULL;
    contours_fifo_t* fifo = (contours_fifo_t*)handle;
    if (handle == NULL) {
        log_error("null handle in %s\n", __func__);
        return NULL;
    }

    pthread_mutex_lock(&fifo->mutex);
    if (fifo->size >= fifo->capability && fifo->strategy == OVERFLOW_STRATEGY_DISCARD)
    {
        node = NULL;
    }
    else
    {
        node = fifo->arr + fifo->windex;
    }
    pthread_mutex_unlock(&fifo->mutex);

    return node;
}

int cfifo_push(fifo_handle_t handle, frame_contours_t* data)
{
    contours_fifo_t* fifo = (contours_fifo_t*)handle;
	frame_contours_t* node = NULL;

	if (handle == NULL || data == NULL) {
		return -1;
	}

	pthread_mutex_lock(&fifo->mutex);

	if(fifo->size >= fifo->capability)
    {
        if (fifo->strategy == OVERFLOW_STRATEGY_DISCARD)
        {
		    log_error("centroid fifo overflow (capability=%d)! discard data according strategy.\n",
                fifo->capability);
		    pthread_mutex_unlock(&fifo->mutex);
		    return -1;
        }
        else
        {
            log_error("centroid fifo overflow (capability=%d)! overwrite oldest data according strategy.\n",
                fifo->capability);
        }
	}

	node = fifo->arr + fifo->windex;
	memcpy(node, data, sizeof(frame_contours_t));

	//offset the next one
	if(++fifo->windex >= fifo->capability) {
		fifo->windex = 0;
	}

	//increase a node
	if(++fifo->size > fifo->capability) {
		fifo->size = fifo->capability;
	}

	pthread_mutex_unlock(&fifo->mutex);
	return 0;
}

int cfifo_size(fifo_handle_t handle)
{
	int cnt;
    contours_fifo_t* fifo = (contours_fifo_t*)handle;

	assert(handle != NULL);
	pthread_mutex_lock(&fifo->mutex);
	cnt = fifo->size;
	pthread_mutex_unlock(&fifo->mutex);
	return cnt;
}

frame_contours_t* cfifo_pop(fifo_handle_t handle)
{
    frame_contours_t* node = NULL;
	contours_fifo_t* fifo = (contours_fifo_t*)handle;
	if (handle == NULL) {
		return NULL;
	}

	pthread_mutex_lock(&fifo->mutex);
	if(fifo->size <= 0){
		pthread_mutex_unlock(&fifo->mutex);
		return NULL;
	}
	node = fifo->arr + fifo->rindex;
	fifo->size--;
	if(++fifo->rindex >= fifo->capability){
		fifo->rindex = 0;
	}
	pthread_mutex_unlock(&fifo->mutex);
	return node;
}

void cfifo_free (fifo_handle_t handle)
{
    contours_fifo_t* fifo = (contours_fifo_t*)handle;
	FREE(fifo->arr);
    FREE(fifo);
}

#ifdef __cplusplus
}
#endif


