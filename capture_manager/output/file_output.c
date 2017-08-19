
#include "../common.h"
#include "output.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    FILE* file;
    int target_frames;
    int saved_frames;
    uint8_t* buf;
    int buf_capability;
    int buf_valid_size;
} file_output_context_t;

static file_output_context_t g_octx;

static int file_open(output_param_t* param)
{
    CLEAR(g_octx);
    g_octx.file = fopen(param->dev, "wb");
    if (g_octx.file == NULL)
    {
        log_error("failed to open or create file %s\n", param->dev);
        return -1;
    }

    return 0;
}

static int file_start()
{
    log_debug("%s is called\n", __func__);
    return 0;
}

static void file_close()
{
}

static int file_write_frame(unsigned char * frame_buf, int frame_size)
{
    return 0;
}

static int file_get_next_buf(unsigned char **buf, int *buf_size)
{
    return 0;
}


output_device_t file_output_device = {
    .name = "fifo output",
    .open = file_open,
    .start = file_start,
    .close = file_close,
    .write_frame = file_write_frame,
    .get_next_buf = file_get_next_buf
};

#ifdef __cplusplus
}
#endif

