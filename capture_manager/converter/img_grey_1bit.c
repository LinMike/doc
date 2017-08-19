#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "converter.h"

static int check_support(input_param_t* iparam, output_param_t* oparam)
{
    return 1; //always support
}

static int grey1_to_yuv420_backup(frame_buffer_t* src, frame_buffer_t* dst)
{
    register int i, j;
    register unsigned char val;

    if (dst->size < (src->size * 8 * 3 / 2))
    {
        log_error("%s: dst->size is too small, src->size=%d, dst->size=%d\n", __func__, src->size, dst->size);
        return -1;
    }

    memset(dst->buf, 0, src->size * 8);

    for (i = 0, j = 0; i < src->size; ++i, j+=8)
    {
        val = src->buf[i];
        if (val == 0) {
            continue;
        }

        dst->buf[j] = (val & 0x01) ? 0xFF : 0x00;
        dst->buf[j+1] = (val & 0x02) ? 0xFF : 0x00;
        dst->buf[j+2] = (val & 0x04) ? 0xFF : 0x00;
        dst->buf[j+3] = (val & 0x08) ? 0xFF : 0x00;
        dst->buf[j+4] = (val & 0x10) ? 0xFF : 0x00;
        dst->buf[j+5] = (val & 0x20) ? 0xFF : 0x00;
        dst->buf[j+6] = (val & 0x40) ? 0xFF : 0x00;
        dst->buf[j+7] = (val & 0x80) ? 0xFF : 0x00;
    }

    int tmp = src->size * 8 / 2;
    memset(dst->buf + tmp * 2, 0x80, tmp);
    return 0;
}

/**
 * grey1_to_yuv420_fast converts the input grey 1bit image into yuv420 image with faster speed
 */
static int grey1_to_yuv420_fast(frame_buffer_t* src, frame_buffer_t* dst)
{
    register int i, j;
    register uint32_t val;
    register uint8_t* ptr;

    if (dst->size < (src->size * 8 * 3 / 2))
    {
        log_error("%s: dst->size is too small, src->size=%d, dst->size=%d\n", __func__, src->size, dst->size);
        return -1;
    }

    if (((int)src->buf % 4) != 0)
    {
        log_error("%s: input buffer (%p) is not 32bit aligned!\n", __func__, src->buf);
    }

    // Fill the Y planer
    memset(dst->buf, 0, src->size * 8);

    for (i = 0, j = 0; i < src->size; i+=4, j+=32)
    {
        val = *((uint32_t*)(src->buf + i));
        if (val == 0)
        {
            continue;
        }
        else if (val == 0xffffffff)
        {
            memset(dst->buf + j, 0xff, 32);
        }
        else
        {
            ptr = dst->buf + j;

            ptr[ 0] = (val & 0x00000001) ? 0xFF : 0x00;
            ptr[ 1] = (val & 0x00000002) ? 0xFF : 0x00;
            ptr[ 2] = (val & 0x00000004) ? 0xFF : 0x00;
            ptr[ 3] = (val & 0x00000008) ? 0xFF : 0x00;
            ptr[ 4] = (val & 0x00000010) ? 0xFF : 0x00;
            ptr[ 5] = (val & 0x00000020) ? 0xFF : 0x00;
            ptr[ 6] = (val & 0x00000040) ? 0xFF : 0x00;
            ptr[ 7] = (val & 0x00000080) ? 0xFF : 0x00;
            ptr[ 8] = (val & 0x00000100) ? 0xFF : 0x00;
            ptr[ 9] = (val & 0x00000200) ? 0xFF : 0x00;
            ptr[10] = (val & 0x00000400) ? 0xFF : 0x00;
            ptr[11] = (val & 0x00000800) ? 0xFF : 0x00;
            ptr[12] = (val & 0x00001000) ? 0xFF : 0x00;
            ptr[13] = (val & 0x00002000) ? 0xFF : 0x00;
            ptr[14] = (val & 0x00004000) ? 0xFF : 0x00;
            ptr[15] = (val & 0x00008000) ? 0xFF : 0x00;
            ptr[16] = (val & 0x00010000) ? 0xFF : 0x00;
            ptr[17] = (val & 0x00020000) ? 0xFF : 0x00;
            ptr[18] = (val & 0x00040000) ? 0xFF : 0x00;
            ptr[19] = (val & 0x00080000) ? 0xFF : 0x00;
            ptr[20] = (val & 0x00100000) ? 0xFF : 0x00;
            ptr[21] = (val & 0x00200000) ? 0xFF : 0x00;
            ptr[22] = (val & 0x00400000) ? 0xFF : 0x00;
            ptr[23] = (val & 0x00800000) ? 0xFF : 0x00;
            ptr[24] = (val & 0x01000000) ? 0xFF : 0x00;
            ptr[25] = (val & 0x02000000) ? 0xFF : 0x00;
            ptr[26] = (val & 0x04000000) ? 0xFF : 0x00;
            ptr[27] = (val & 0x08000000) ? 0xFF : 0x00;
            ptr[28] = (val & 0x10000000) ? 0xFF : 0x00;
            ptr[29] = (val & 0x20000000) ? 0xFF : 0x00;
            ptr[30] = (val & 0x40000000) ? 0xFF : 0x00;
            ptr[31] = (val & 0x80000000) ? 0xFF : 0x00;
        }
    }

    //Fill the UV planer
    //set UV to below value will make sure the output image is grey image
    int tmp = src->size * 8 / 2;
    memset(dst->buf + tmp * 2, 0x80, tmp);
    return 0;
}

converter_t converter_grey1_to_yuv420 = {
    .name = "Grey 1bit to YUV420",
    .convert = grey1_to_yuv420_fast,
    .check_support = check_support
};

