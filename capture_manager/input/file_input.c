#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "input.h"

static FILE* g_file = NULL;
static int g_frame_size;
static int g_file_size;
static int g_next_file_offset = 0;
static uint8_t* g_frame_buf = NULL;
static long g_time_last = 0;
static long g_frame_interval = 33;

static input_param_t g_param;

static int file_open(input_param_t* param)
{
    LOG_FUNC_CALLED;

    log_debug("%s, framesize=%d\n", param->dev, param->frame_size);

    if (param == NULL)
        return -1;

    memcpy(&g_param, param, sizeof(input_param_t));

    g_file = fopen(g_param.dev, "rb");
    if (g_file == NULL)
    {
        log_error("%s: fail to open file %s\n", __func__, g_param.dev);
        return -1;
    }

    fseek(g_file, 0, SEEK_END);
    g_file_size = ftell(g_file);
    fseek(g_file, 0, SEEK_SET);

    g_frame_size = g_param.frame_size;
    g_frame_interval = 1000/g_param.framerate;

    log_debug("g_frame_size=%d, file_size=%d\n", g_frame_size, g_file_size);

    g_next_file_offset = 0;
    return 0;
}

static int file_start()
{
    LOG_FUNC_CALLED
    g_frame_buf = (uint8_t*)malloc(g_frame_size);
    if (g_frame_buf == NULL)
    {
        log_error("malloc for frame buffer failed!\n");
        return -1;
    }
    return 0;
}

static void file_close()
{
    FREE(g_frame_buf);
    fclose(g_file);
    g_file = NULL;
    g_time_last = 0;
}

static int file_read_frame(uint8_t** framebuf, int* framesize)
{
    long tcur = get_time_ms();
    if (g_time_last != 0 && (tcur - g_time_last) < g_frame_interval)
    {
        return 0; //time is not reached for next frame
    }
    g_time_last = tcur;
    ////log_debug("framebuf=%p, size=%d, next=%d\n", framebuf, g_frame_size, g_next_file_offset);

    fseek(g_file, g_next_file_offset, SEEK_SET);
    int nread = fread(g_frame_buf, 1, g_frame_size, g_file);
    if (nread != g_frame_size)
    {
        log_error("read current frame from file failed!\n");
        return -1;
    }

    g_next_file_offset += g_frame_size;
    if (g_next_file_offset >= g_file_size)
    {
        g_next_file_offset = 0;
    }

    *framebuf = g_frame_buf;
    *framesize = g_frame_size;
    return g_frame_size;
}


input_device_t file_input_device = {
    .name = "file input",
    .start = file_start,
    .open = file_open,
    .close = file_close,
    .read_frame = file_read_frame
};


