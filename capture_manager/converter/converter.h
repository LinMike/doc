#ifdef __cplusplus
extern "C" {
#endif


#ifndef __CONVERTER_H__
#define __CONVERTER_H__

#include "../common.h"
#include "../input/input.h"
#include "../output/output.h"

typedef struct converter {
    char* name;
    int (*open)(frame_buffer_t* src, frame_buffer_t* dst);
    int (*close)();
    int (*convert)(frame_buffer_t* src, frame_buffer_t* dst);
    int (*check_support)(input_param_t*, output_param_t*);
} converter_t;

extern converter_t converter_yuv420_color_to_grey;
extern converter_t converter_grey8_to_yuv420;
extern converter_t converter_grey1_to_yuv420;
extern converter_t converter_contour_to_fifo;
extern converter_t converter_contour_to_yuv420;
extern converter_t converter_dualcontour8_to_yuv420;


#endif

#ifdef __cplusplus
}
#endif
