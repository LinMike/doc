
#ifndef _CONTOUR_FIFO_H_
#define _CONTOUR_FIFO_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "../common.h"
#include "../contour.h"

/**********************************************
 * Common Contour Fifo
 *********************************************/
typedef void* fifo_handle_t;

fifo_handle_t cfifo_create(const int capacity);

frame_contours_t* cfifo_next(fifo_handle_t handle);

int cfifo_push(fifo_handle_t handle, frame_contours_t* data);

frame_contours_t* cfifo_pop(fifo_handle_t handle);

int cfifo_size(fifo_handle_t handle);

void cfifo_free(fifo_handle_t handle);

/**********************************************
 * Gloal Contour FIFO instance
 *********************************************/
fifo_handle_t global_fifo_get();
int global_fifo_open();
void global_fifo_close();

#ifdef __cplusplus
}
#endif

#endif


