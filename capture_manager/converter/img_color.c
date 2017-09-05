#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "converter.h"

static int yuv420_color_to_grey(frame_buffer_t* src, frame_buffer_t* dst)
{
    if (dst->size < src->size)
    {
        log_error("%s: dst->size is too small, src->size=%d, dst->size=%d\n", __func__, src->size, dst->size);
        return -1;
    }

    int offset = src->size * 2 / 3;
    memcpy(dst->buf, src->buf, offset);
    memset(dst->buf + offset, 0x80, src->size - offset);
    return 0;
}

converter_t converter_yuv420_color_to_grey = {
    .name = "YUV420 Color to Grey",
    .convert = yuv420_color_to_grey,
    .check_support = NULL
};

