#ifdef __cplusplus
extern "C" {
#endif

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <stdio.h>

#define SUCCESS (0)
#define FAILURE (-1)

#define STD_IMAGE_WIDTH     1280
#define STD_IMAGE_HEIGHT    720
#define STD_IMAGE_SIZE      (STD_IMAGE_WIDTH * STD_IMAGE_HEIGHT)

#define MAX_CAMERA_NUM (2)

enum CameraId {
    CAMERA_ANY = 0, //Used in some cases that cannot determine the camera ID, for example, 1bit grey input
    CAMERA_LEFT = 1,
    CAMERA_RIGHT = 2,
};

enum CapMode {
    CAP_MODE_GREY_1BIT = 1,
    CAP_MODE_GREY_8BIT = 2,
    CAP_MODE_COLOR_8BIT = 3,
    CAP_MODE_OV5640 = 4, //This has to be aligned with ov5640's 1280x720 setting
    ___PLACEHOLDER = 5,
    CAP_MODE_CONTOUR = 6,
    CAP_MODE_BACKGROUND_SUB = 7,
    CAP_MODE_OPEN = 8,
    CAP_MODE_DUAL_CAMERA_COLOR = 9
};


typedef struct frame_buffer {
    int size;
    int width;
    int height;
    uint8_t* buf;
    enum CameraId camera; //After converter, the camera will be set by parsing the input frame data
} frame_buffer_t;

typedef struct point {
    int x;
    int y;
    int offset;
} point_t;

#define FREE(p) { free(p); p = NULL; }
#define CLEAR(x) memset(&(x), 0, sizeof(x))

enum LogLevel
{
    LOG_CRITICAL = 0,
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
};

static int log_level = LOG_DEBUG;
#define log_set_level(level) log_level = (level)

#define log(level, fmt, arg...) \
    if (level <= log_level) \
    fprintf((level <= LOG_ERROR ? stderr : stderr), fmt, ##arg)

#define log_critical(fmt, arg...) log(LOG_CRITICAL, fmt, ##arg)
#define log_error(fmt, arg...) log(LOG_ERROR, fmt, ##arg)
#define log_warn(fmt, arg...) log(LOG_WARN, fmt, ##arg)
#define log_info(fmt, arg...) log(LOG_INFO, fmt, ##arg)
#define log_debug(fmt, arg...) log(LOG_DEBUG, fmt, ##arg)

#define LOG_FUNC_CALLED log_debug("%s is called!\n", __func__);

void dump_byte_array(const uint8_t* const data, const int size);
void dump_data_to_file(uint8_t* data, const int size, const char* filename);
void dump_data_to_file_autoname(uint8_t* data, int size, const char* suffix);

char* pixfmt_string(const uint32_t fmt, char* str);

void get_local_time_str(char* buf, const char* suffix);
long get_time_ms();

//alarm
int alarm_on(int duration);
void alarm_ctrl();

typedef void* handle_t;

#endif

#ifdef __cplusplus
}
#endif

