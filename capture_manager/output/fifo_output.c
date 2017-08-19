
#include "../utils/contour_fifo.h"
#include "../common.h"
#include "output.h"

#ifdef __cplusplus
extern "C" {
#endif

static fifo_handle_t g_fifo = NULL;

static int fifo_open(output_param_t* param)
{
    g_fifo = global_fifo_get(); //use the global fifo rather than private one
    if (g_fifo == NULL) {
        log_error("Fail to get the global fifo!\n");
        return -1;
    }

    return 0;
}

static int fifo_start()
{
    log_debug("%s is called\n", __func__);
    return 0;
}

static void fifo_close()
{
    //The global fifo is managed by external, so don't need to free it here.
    //cfifo_free(g_fifo);
    g_fifo = NULL;
}

static int fifo_write_frame(unsigned char * frame_buf, int frame_size)
{
    //the input frame_buf has to be the frame_centroid_t
    frame_contours_t* data = (frame_contours_t*)frame_buf;
    return cfifo_push(g_fifo, data);
}

static int fifo_get_next_buf(unsigned char **buf, int *buf_size)
{
    frame_contours_t* next_avaiable = cfifo_next(g_fifo);
    if (next_avaiable == NULL) {
        log_error("the next avaiable centroid buffer is null, may be overflow!\n");
        *buf = NULL;
        *buf_size = 0;
        return -1;
    }
    *buf = (unsigned char*)next_avaiable;
    *buf_size = sizeof(frame_contours_t);
    return 0;
}


output_device_t fifo_output_device = {
    .name = "fifo output",
    .open = fifo_open,
    .start = fifo_start,
    .close = fifo_close,
    .write_frame = fifo_write_frame,
    .get_next_buf = fifo_get_next_buf
};

#ifdef __cplusplus
}
#endif

