#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#ifndef __CAPTURE_MANAGER_H__
#define __CAPTURE_MANAGER_H__

enum InputDevice {
    DEV_INPUT_FPGA = 0,
    DEV_INPUT_FILE = 1,
    DEV_INPUT_MOCK_CONTOUR8 = 2,
    DEV_INPUT_MOCK_CONTOUR4 = 3
};

enum OutputDevice {
    DEV_OUTPUT_HDMI = 0,
    DEV_OUTPUT_FIFO = 1,
    DEV_OUTPUT_FILE = 2,
    DEV_OUTPUT_FB = 3,
    DEV_OUTPUT_UDP = 4,
    DEV_OUTPUT_NOOP = 5,
};

typedef struct capmgr_config
{
    int    enable_preview;
    int     fr_stat_interval;
    enum CapMode cap_mode;
    int     fr_output;
    int fr_input;
    enum InputDevice input_dev;
    enum OutputDevice output_dev;
    enum CameraId camera;
} capmgr_config_t;


int capmgr_open(capmgr_config_t* config);
int capmgr_mainloop();
int capmgr_close();
int capmgr_stop();
int capmgr_pause();
int capmgr_save(int frame_count, int skip_count, char* file_name);
int capmgr_set_camera(enum CameraId id);


//@param interval {int} 0 means stop framereate stat, otherwise means stat interval
int capmgr_framerate_stat(int interval);

int capmgr_enable_preview();

void workaround();

#endif

#ifdef __cplusplus
}
#endif

