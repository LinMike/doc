#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "converter.h"

static int grey8_to_yuv420(frame_buffer_t* src, frame_buffer_t* dst)
{
    if (src->size * 3 / 2 > dst->size)
    {
        log_error("%s: dst->size is too small, src->size=%d, dst->size=%d\n", __func__, src->size, dst->size);
        return -1;
    }

    //fill the Y planar
    memcpy(dst->buf, src->buf, src->size);

    //fill the UV planar
    //change UV to 0x80 will change the image to grey level
    memset(dst->buf + src->size, 0x80, src->size / 2);
    return 0;
}

converter_t converter_grey8_to_yuv420 = {
    .name = "Grey 8bits to YUV420",
    .convert = grey8_to_yuv420,
    .check_support = NULL
};

