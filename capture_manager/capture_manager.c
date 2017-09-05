#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "capture_manager.h"
#include "common.h"
#include "utils/dropframe.h"
#include "utils/frame_recorder.h"
#include "input/input.h"
#include "output/output.h"
#include "converter/converter.h"
#include "contour.h"

#define ORIGIN_FRAME_WIDTH  1280
#define ORIGIN_FRAME_HEIGHT 720
#define OUT_FRAME_SIZE (ORIGIN_FRAME_WIDTH * ORIGIN_FRAME_HEIGHT * 3 / 2 * 2) //for yuv420p and dual cameras

static input_device_t* g_input_device = NULL;
static output_device_t* g_output_device = NULL;
static converter_t *g_converter = NULL;
static frame_buffer_t g_input_buffer;
static frame_buffer_t g_output_buffer;

static capmgr_config_t g_capmgr_config;
static input_param_t g_input_param;
static int g_input_fd;
static output_param_t g_vout_param;
static pthread_mutex_t g_config_mutex  = PTHREAD_MUTEX_INITIALIZER;

static uint8_t *g_buf = NULL;
static int g_bufsize = 0;

static DropFrameContex_t g_dropframe_ctx;
static handle_t g_frame_recorder = NULL;

////////////////////////////////////////////////////////////////////////
// Capture Manager Control Flags
////////////////////////////////////////////////////////////////////////

static int g_control_flag = 0x00;
static pthread_mutex_t g_control_mutex  = PTHREAD_MUTEX_INITIALIZER;

#define FLAG_STOP       0x01
#define FLAG_PAUSE      0x02

static int is_stop()
{
    int flag;
    pthread_mutex_lock(&g_control_mutex);
    flag = g_control_flag;
    pthread_mutex_unlock(&g_control_mutex);
    return (flag & FLAG_STOP);
}

static int is_pause()
{
    int flag;
    pthread_mutex_lock(&g_control_mutex);
    flag = g_control_flag;
    pthread_mutex_unlock(&g_control_mutex);
    return (flag & FLAG_PAUSE);
}

int capmgr_stop()
{
    pthread_mutex_lock(&g_control_mutex);
    g_control_flag |= FLAG_STOP;
    pthread_mutex_unlock(&g_control_mutex);
}

int capmgr_pause()
{
    pthread_mutex_lock(&g_control_mutex);
    g_control_flag |= FLAG_PAUSE;
    pthread_mutex_unlock(&g_control_mutex);
}


int capmgr_set_camera(enum CameraId id)
{
    int ret;
    pthread_mutex_lock(&g_control_mutex);
    ret = reg_set_source_format(id, g_capmgr_config.cap_mode);
    pthread_mutex_unlock(&g_control_mutex);
    return ret;
}

static int convert_frame_format(frame_buffer_t* src, frame_buffer_t* dst)
{
    if (g_converter!= NULL)
    {
        if (g_converter->convert(src, dst) < 0)
        {
            //log_error("The converter fails to convert frame!\n");
            return -1;
        }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////
// Framerate Statistics
////////////////////////////////////////////////////////////////////////

static long g_fr_stat_interval = 0; //defaultly stop

static void stat_framerate(int inframe_count, int outframe_count)
{
    long interval = (long)g_capmgr_config.fr_stat_interval;

    if (interval <= 0) { //means stop framereate stat
        return;
    }

    static long lasttime = 0;
    static long acc_in = 0, acc_out = 0;
    long dur;

    if (lasttime == 0) {
        lasttime = get_time_ms();
    }

    acc_in += inframe_count;
    acc_out += outframe_count;

    dur = (get_time_ms() - lasttime);
    if (dur >= interval) {
        log_info("\n>>> ifr ~= %.1f, ofr ~= %.1f\n",
            (double)(acc_in  * 1000) / (double)dur,
            (double)(acc_out * 1000) / (double)dur );
        acc_in = acc_out = 0;
        lasttime = get_time_ms();
    }
}

int capmgr_framerate_stat(int interval)
{
    g_capmgr_config.fr_stat_interval = interval;
    return 0;
}



////////////////////////////////////////////////////////////////////////
// Save Frames
////////////////////////////////////////////////////////////////////////

static int g_save_frame_count = 0;
static char g_save_file_name[64] = {0};
static FILE* g_save_file = NULL;
static pthread_mutex_t g_save_mutex = PTHREAD_MUTEX_INITIALIZER;

static void save_frame(unsigned char* buf, int size)
{
    pthread_mutex_lock(&g_save_mutex);

    frame_recorder_append(g_frame_recorder, buf, size);
    if (frame_recorder_get_state(g_frame_recorder) == RECORDER_STATE_FULL) {
        log_info("All saved frames have been cached, start flushing to filesystem\n");
        frame_recorder_flush(g_frame_recorder, g_save_file_name);
        frame_recorder_reset(g_frame_recorder);
    }

    pthread_mutex_unlock(&g_save_mutex);
}

int capmgr_save(int frame_count, int skip_count, char* file_name)
{
    int ret = -1;
    pthread_mutex_lock(&g_save_mutex);

    if (frame_count <= 0) {
        log_error("frame_count must be a positive integer!\n");
        goto unlock_ret;
    }

    if (frame_recorder_get_state(g_frame_recorder) != RECORDER_STATE_UNSTART) {
        log_error("last frame saving is still ongoing, please try after a while.\n");
        goto unlock_ret;
    }

    if (file_name == NULL) {
        get_local_time_str(g_save_file_name, ".yuv");
    }
    else if (strlen(file_name) >= (sizeof(g_save_file_name) - 1)) {
        log_error("input file saving name is too long!\n");
        goto unlock_ret;
    }
    else {
        strcpy(g_save_file_name, file_name);
    }

    frame_recorder_set_target_frames(g_frame_recorder, frame_count);
    frame_recorder_set_skip_frames(g_frame_recorder, skip_count);
    ret = 0;
unlock_ret:
    pthread_mutex_unlock(&g_save_mutex);
    return ret;
}


////////////////////////////////////////////////////////////////////////
// Main Functions
// Glue capture and video-output together
////////////////////////////////////////////////////////////////////////

int get_out_buf(int frameflag, uint8_t** buf, int* bufsize)
{
    if (g_output_device->get_next_buf != NULL && frameflag && g_capmgr_config.enable_preview) {
        g_output_device->get_next_buf(buf, bufsize);
    } else {
        *buf = g_buf;
        *bufsize = g_bufsize;
    }
    return 0;
}

static int process_frame(unsigned char* inbuf, int insize)
{
    int i;
    int ret;
    static int frame_idx = 0;

    int cur_frame_flag = dropframe_get_flag(&g_dropframe_ctx);

    if (inbuf == NULL || insize <= 0)
    {
        log_error("inbuf is null or insize <= 0\n");
        return - 1;
    }

    save_frame(g_input_buffer.buf, g_input_buffer.size);

    //manually calculate the current frame rate
    stat_framerate(1, 0);

    //Get next avaiable output buffer and edit in place, so we don't need to allocate buffer
    ret = get_out_buf(cur_frame_flag, &g_output_buffer.buf, &g_output_buffer.size);
    if (ret < 0 || g_output_buffer.buf == NULL || g_output_buffer.size == 0)
    {
        log_error("Query the output buffer but get failure, maybe the output device is busy so the buffer is not avaiable!\n");
        return -1;
    }

    //Convert the input frame buffer into output buffer, so that the output frame can be corrected displayed.
    if (convert_frame_format(&g_input_buffer, &g_output_buffer) < 0)
    {
        //log_error("%s: convert_frame_format failed!\n", __func__);
        return -1;
    }

    //Write converted frames to display
    if (g_capmgr_config.enable_preview && cur_frame_flag) {
        g_output_device->write_frame(g_output_buffer.buf, g_output_buffer.size);
        stat_framerate(0, 1);
        //save_frame(g_output_buffer.buf, g_output_buffer.size);
    }

    //Save frame raw data if it is allowed
    //save_frame(g_output_buffer.buf, g_output_buffer.size);
    //save_frame(g_input_buffer.buf, g_input_buffer.size);

    return 0;
}


int capmgr_mainloop()
{
    unsigned char* frame_buf;
    int frame_size;
    int i = 0;
    int fd;

    if (g_input_device->get_fd != NULL)
    {
        fd = g_input_device->get_fd();
    }

    while (1)
    {
        if (is_stop()) break;
        if (is_pause()) continue;

        //log_debug("%d\n", i++);
        for (;;)
        {
            //if (fd > 0)
            if (0)
            {
                fd_set fds;
                struct timeval tv;
                int r;

                FD_ZERO(&fds);
                FD_SET(fd, &fds);

                tv.tv_sec = 2; //Timeout
                tv.tv_usec = 0;

                r = select(fd + 1, &fds, NULL, NULL, &tv);

                if (-1 == r)
                {
                    if (EINTR == errno)
                        continue;
                    log_error("mainloop select error, errorno=%d\n", errno);
                    break;
                }

                if (0 == r)
                {
                    log_error("mainloop select timeout\n");
                    break;
                }

                if (g_input_device->read_frame(&g_input_buffer.buf, &g_input_buffer.size))
                {
                    if (process_frame(g_input_buffer.buf, g_input_buffer.size) < 0)
                    {
                        //log_error("process frame fail!\n");
                        //return -1;
                    }
                    break;
                }
            }
            else
            {
                if (g_input_device->read_frame(&g_input_buffer.buf, &g_input_buffer.size))
                {
                    if (process_frame(g_input_buffer.buf, g_input_buffer.size) < 0)
                    {
                        //log_error("process frame fail!\n");
                        //return -1;
                    }
                }
            }

            alarm_ctrl();

            /* EAGAIN - continue select loop. */
        }
    }
    return 0;
}

static void calc_frame_size(input_param_t* param)
{
    if (param->pixfmt == V4L2_PIX_FMT_YUV420)
    {
        param->frame_size = param->width * param->height * 3 / 2;
    }
    else if (param->pixfmt == V4L2_PIX_FMT_YUYV)
    {
        param->frame_size = param->width * param->height * 2;
    }
    else
    {
        param->frame_size = param->width * param->height * 3; //use maximum size for unknow format
    }
}

static int parse_mode(enum CameraId camera, enum CapMode mode, const enum InputDevice idev, input_param_t* iparam, enum OutputDevice odev)
{
    //This has to be aligned with driver
    iparam->camera_mode = (int)mode;

    if (idev == DEV_INPUT_FPGA && reg_set_source_format(camera, mode) < 0)
    {
        return -1;
    }

    switch(mode)
    {
        case CAP_MODE_CONTOUR:
            iparam->width = CONTOUR_FRAME_WIDTH/2;
            iparam->height = CONTOUR_FRAME_HEIGHT*2;
            iparam->pixfmt = V4L2_PIX_FMT_YUYV;
            if (odev == DEV_OUTPUT_FIFO)
                g_converter = &converter_contour_to_fifo;
            else
                g_converter = &converter_contour_to_yuv420;
            break;
        case CAP_MODE_GREY_1BIT:
        case CAP_MODE_BACKGROUND_SUB:
        case CAP_MODE_OPEN:
            iparam->width = 800;
            iparam->height = 72;
            g_converter = &converter_grey1_to_yuv420;
            iparam->pixfmt = V4L2_PIX_FMT_YUYV;
            break;
        case CAP_MODE_GREY_8BIT:
            iparam->width = 1280;
            iparam->height = 480;
            g_converter = &converter_grey8_to_yuv420;
            iparam->pixfmt = V4L2_PIX_FMT_YUV420;
            break;
        case CAP_MODE_COLOR_8BIT:
        case CAP_MODE_OV5640:
            iparam->width = 1280;
            iparam->height = 720;
            g_converter = &converter_yuv420_color_to_grey;
            iparam->pixfmt = V4L2_PIX_FMT_YUV420;
            break;
        case CAP_MODE_DUAL_CAMERA_COLOR:
            iparam->width = 1280;
            iparam->height = 1440;
            g_converter = NULL;
            iparam->pixfmt = V4L2_PIX_FMT_YUV420;
            break;
         default:
            log_error("unrecongized capture mode %d\n", mode);
            return -1;
    }

    calc_frame_size(iparam);
    g_input_buffer.width = iparam->width;
    g_input_buffer.height = iparam->height;
    g_input_buffer.size = iparam->frame_size;

    log_debug("calculated frame size=%d\n", iparam->frame_size);
    return 0;
}

void parse_input_device(enum InputDevice dev)
{
    switch(dev)
    {
        case DEV_INPUT_FPGA:
            g_input_device = &fpga_input_device;
            break;
        case DEV_INPUT_FILE:
            g_input_device = &file_input_device;

            break;
        case DEV_INPUT_MOCK_CONTOUR8:
            g_input_device = &mock_contour8_input_device;
            break;
        case DEV_INPUT_MOCK_CONTOUR4:
            log_error("The input contour4 is not supported now!\n");
            //g_input_device = &mock_contour4_input_device;
            break;
        default:
            log_error("unsupported input device (%d)\n", dev);
            exit(-1);
            break;
    }
}

void parse_output_device(enum OutputDevice dev)
{
    switch(dev)
    {
        case DEV_OUTPUT_HDMI:
            g_output_device = &hdmi_v4l2_output_device;
            break;
        case DEV_OUTPUT_FIFO:
            g_output_device = &fifo_output_device;
            break;
        case DEV_OUTPUT_FB:
            g_output_device = &hdmi_fb_output_device;
            break;
        case DEV_OUTPUT_UDP:
            g_output_device = &udp_output_device;
            break;
        case DEV_OUTPUT_NOOP:
            g_output_device = &noop_output_device;
            break;
        case DEV_OUTPUT_FILE:
        default:
            log_error("unsupported output device (%d)\n", dev);
            exit(-1);
            break;
    }
}

int set_input_dev_name(char* target_dev, const enum InputDevice dev, const enum CapMode mode)
{
    int flag = 1;
    switch(dev)
    {
    case DEV_INPUT_FPGA:
        strcpy(target_dev, "/dev/video1");
        break;
    case DEV_INPUT_FILE:
        if (mode == CAP_MODE_GREY_1BIT)
        {
            strcpy(target_dev, "mock.y1");
        }
        else if (mode == CAP_MODE_GREY_8BIT)
        {
            strcpy(target_dev, "mock.y8");
        }
        else if (mode == CAP_MODE_CONTOUR)
        {
            strcpy(target_dev, "mock.contour");
        }
        else if (mode == CAP_MODE_COLOR_8BIT || mode == CAP_MODE_OV5640 || mode == CAP_MODE_DUAL_CAMERA_COLOR)
        {
            strcpy(target_dev, "mock.yuv420p");
        }
        else
        {
            log_error("Currently doesn't support the mock input for mode (%d)\n", mode);
            return -1;
        }
        break;
    default:
        flag = 0;
        break;
    }

    if (flag)
    {
        log_info("Using input device: %s\n", target_dev);
    }
}

int capmgr_open(capmgr_config_t* config)
{
    int ret = -1;

    if (config != NULL)
    {
        log_info("input_dev:%d\n", config->input_dev);
        log_info("output_dev:%d\n", config->output_dev);
        log_info("mode: %d\n", config->cap_mode);
        memcpy(&g_capmgr_config, config, sizeof(capmgr_config_t));
    }
    else
    {
        g_capmgr_config.enable_preview = 1;
        g_capmgr_config.fr_stat_interval = 1000;
        g_capmgr_config.cap_mode = CAP_MODE_CONTOUR;
        g_capmgr_config.input_dev = DEV_INPUT_FPGA;
        g_capmgr_config.output_dev = DEV_OUTPUT_FIFO;
        g_capmgr_config.fr_input = 60;
        g_capmgr_config.fr_output = 60;
    }

    parse_input_device(g_capmgr_config.input_dev);
    parse_output_device(g_capmgr_config.output_dev);

    if (g_capmgr_config.input_dev == DEV_INPUT_FPGA && com_open() < 0)
    {
        log_error("open communication fail!\n");
        goto ret_close;
    }

    if (g_capmgr_config.output_dev == DEV_OUTPUT_FIFO && global_fifo_open() < 0)
    {
        log_error("initialize the global fifo fails!\n");
        goto ret_close;
    }

    log_debug("global fifo open succeed\n");

    CLEAR(g_input_param);
    if (parse_mode(config->camera, g_capmgr_config.cap_mode, config->input_dev,
            &g_input_param, config->output_dev))
    {
        goto ret_close;
    }

    if (set_input_dev_name(g_input_param.dev, config->input_dev, config->cap_mode) < 0)
    {
        goto ret_close;
    }
    g_input_param.framerate = g_capmgr_config.fr_input;

    g_input_fd = g_input_device->open(&g_input_param);
    if (g_input_fd < 0) {
        log_error("failed to open input device!\n");
        goto ret_close;
    }

    g_output_buffer.width = ORIGIN_FRAME_WIDTH;
    g_output_buffer.height = ORIGIN_FRAME_HEIGHT;

    if (g_capmgr_config.enable_preview) {
        CLEAR(g_vout_param);
        strcpy(g_vout_param.dev, "/dev/video18");
        g_vout_param.source_width = ORIGIN_FRAME_WIDTH;
        g_vout_param.source_height = ORIGIN_FRAME_HEIGHT;
        g_vout_param.display_width = 1920;
        g_vout_param.display_height = 1080;
        g_vout_param.rotate = 0;//180;
        g_vout_param.vertical_flip = 0;
        g_vout_param.horizontal_flip = 0;//1;

        ret = g_output_device->open(&g_vout_param);
        if (ret < 0) {
            log_error("failed to open vout!\n");
            goto ret_close;
        }

        dropframe_init(&g_dropframe_ctx, g_capmgr_config.fr_input, g_capmgr_config.fr_output);
        dropframe_dump_flag(&g_dropframe_ctx);
    }

    //A general frame buffer for frame skipping
    g_bufsize = OUT_FRAME_SIZE;
    g_buf = (uint8_t*)malloc(g_bufsize);
    if (g_buf == NULL) {
        log_error("malloc g_buf failed!\n");
        goto ret_close;
    }

    log_info("================================\n");
    log_info("    input: %s\n", g_input_device->name);
    log_info("   output: %s\n", g_output_device->name);
    log_info("converter: %s\n", g_converter != NULL ? g_converter->name : "NULL");
    log_info("================================\n");

    if (g_converter != NULL)
    {
        if (g_converter->check_support != NULL && !g_converter->check_support(&g_input_param, &g_vout_param))
        {
            log_error("The converter is not comptible with input & output setting!\n");
            goto ret_close;
        }

        if (g_converter->open != NULL && g_converter->open(&g_input_buffer, &g_output_buffer) < 0)
        {
            log_error("Failed to open the converter!\n");
            goto ret_close;
        }
    }

    g_frame_recorder = frame_recorder_new(0);
    if (g_frame_recorder == NULL) {
        goto ret_close;
    }

    ret = g_input_device->start();
    if (ret < 0) goto ret_close;

    if (g_capmgr_config.enable_preview) {
        ret = g_output_device->start();
        if (ret < 0) goto ret_close;
    }

    log_debug("Input/Output has been started!\n");

    return 0;

ret_close:
    capmgr_close();
    return ret;
}

int capmgr_close()
{
    g_input_device->close();
    if (g_capmgr_config.enable_preview) {
        g_output_device->close();
    }
    FREE(g_buf);

    if (g_converter != NULL && g_converter->close != NULL)
    {
        g_converter->close();
    }

    global_fifo_close();
    com_close();
    frame_recorder_free(g_frame_recorder);

    return 0;
}

int capmgr_enable_preview()
{
    g_capmgr_config.enable_preview = 1;
    return 0;
}

//目前有一个已知的问题, ARM板子开机后没法直接用FrameBuffer显示，需要提前运行一个hdmi测试程序才行。
//目前还没有找到最终的原因，但是测试后下面的方案可以临时性的解决这个问题。
//以下的Workaround是临时以v4l2的方式打开一下HDMI设备，随便写一点数据，然后关闭设备
void workaround()
{
    uint8_t data;
    hdmi_v4l2_output_device.start();
    hdmi_v4l2_output_device.write_frame(&data, 1);
    hdmi_v4l2_output_device.close();
}

