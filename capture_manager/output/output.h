#ifdef __cplusplus
extern "C" {
#endif

#ifndef __VIDEO_OUTPUT_H__
#define __VIDEO_OUTPUT_H__

#include <stdint.h>
#include "../common.h"


typedef struct output_param {
    char dev[32];
    int source_width;
    int source_height;
    int display_width;
    int display_height;
    int rotate;
    int vertical_flip;
    int horizontal_flip;
    enum CameraId camera; //Only used for video output
} output_param_t;

typedef struct output_device {
    char* name;

    int (*open)(output_param_t*);
    int (*start)();
    void (*close)();
    int (*write_frame)(uint8_t*, int);
    int (*get_next_buf)(uint8_t**, int*);
} output_device_t;


extern output_device_t hdmi_v4l2_output_device;
extern output_device_t fifo_output_device;
extern output_device_t hdmi_fb_output_device;
extern output_device_t udp_output_device;
extern output_device_t noop_output_device;

#endif

#ifdef __cplusplus
}
#endif

