#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CAPTURE_MANAGER_INPUT_H__
#define __CAPTURE_MANAGER_INPUT_H__

#include "../common.h"
#include <linux/videodev2.h>


typedef struct input_param {
    char dev[32];
    int framerate;

    //Private, user doesn't need to set below parameters
    int width;
    int height;
    int camera_mode;
    uint32_t pixfmt;
    int frame_size;
} input_param_t;

typedef struct input_device {
    char* name;

    int (*open)(input_param_t*);
    int (*start)();
    void (*close)();
    int (*read_frame)(uint8_t**, int*);
    int (*get_fd)();
} input_device_t;


enum MoveType {
    MOVE_UP_DOWN,
    MOVE_LEFT_RIGHT,
    MOVE_DIAG_45,
    MOVE_DIAG_135,
    MOVE_ZIGZAG,
    MOVE_SQAURE,
    MOVE_NONE
};

typedef struct shape_context {
    int start_x;
    int start_y;
    int width;
    uint8_t* buf;

    //movement
    int x;
    int y;
    enum MoveType move_type;
    int move_distance;
    int dir;
    int move_cnt;
} shape_context_t;

typedef struct shape {
    const char* name;
    shape_context_t* (*create)(shape_context_t* ctx);
    int (*draw)(shape_context_t*);
} shape_t;

extern input_device_t fpga_input_device;
extern input_device_t file_input_device;
extern input_device_t mock_contour8_input_device;

#endif

#ifdef __cplusplus
}
#endif
