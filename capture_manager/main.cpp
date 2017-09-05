#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "common.h"
#include "capture_manager.h"
#include "com/eim.h"
#include "com/com.h"

#include <getopt.h>

static capmgr_config_t config;
static int g_save_frame_count = 0;
static int g_skip_frame_count = 0;

static void usage_i2c(FILE*fp, int argc, char** argv)
{
    fprintf(fp,
         "Usage: %s i2c\n\n"
         "Version 1.0\n"
         "> I2C Read:\n"
         "  %s i2c r <register>\n"
         ""
         "> I2C Write:\n"
         "  %s i2c w <register> <data>\n"
         ""
         "> I2C Test:\n"
         "  %s i2c t\n"
         "",
         argv[0], argv[0], argv[0]
     );
}

static void usage_reg(FILE*fp, int argc, char** argv)
{
    fprintf(fp,
         "Usage: %s reg\n\n"
         "Version 1.0\n"
         "> Register Read:\n"
         "  %s reg r <register>\n"
         ""
         "> Register Write:\n"
         "  %s reg w <register> <data>\n"
         ""
         "> Register Test:\n"
         "  %s reg t\n"
         "",
         argv[0], argv[0], argv[0]
     );
}


static void usage(FILE *fp, int argc, char **argv)
{
    fprintf(fp,
         "Usage: %s [options]\n\n"
         "Version 1.0\n"
         "Options:\n"
         "-m | --mode mode     Capture mode, %d:contour, %d: dual cameras, %d:bg_sub, %d:open, %d:grey-1bit, %d:grey-8bit, %d:color-8bit, %d:ov5640 [%d]\n"
         "-i | --input dev     Select input device, %d:fpga, %d:file, %d:mock contour8\n"
         "-o | --output dev    Select output device, %d:hdmi, %d:fifo, %d:framebuffer, %d:UDP, %d:Noop\n"
         "-p | --preview 0/1   Enable video preview via HDMI [%d]\n"
         "-c | --camera 0/1/2  Select the camera. Any:%d, Left:%d, Right%d [%d]\n"
         "-f | --ofr fr        Output framerate [%d]\n"
         "-g | --ifr fr        Input framerate [%d]\n"
         "-t | --sfr time      The time interval of framerate statistics [%d]ms\n"
         "-s | --save count    Specify how many frames to save (default is 0, max is 100)\n"
         "-k | --skip count    Specify how many frames need be skipped before starting save (default is 0)\n"
         "-v | --verbose       Verbose output\n"
         "-h | --help          Print help message\n"
         "",
         argv[0],

         CAP_MODE_CONTOUR, CAP_MODE_DUAL_CAMERA_COLOR, CAP_MODE_BACKGROUND_SUB, CAP_MODE_OPEN, CAP_MODE_GREY_1BIT, CAP_MODE_GREY_8BIT, CAP_MODE_COLOR_8BIT, CAP_MODE_OV5640, config.cap_mode,
         DEV_INPUT_FPGA, DEV_INPUT_FILE, DEV_INPUT_MOCK_CONTOUR8,
         DEV_OUTPUT_HDMI, DEV_OUTPUT_FIFO, DEV_OUTPUT_FB, DEV_OUTPUT_UDP, DEV_OUTPUT_NOOP,
         config.enable_preview,
         CAMERA_ANY, CAMERA_LEFT, CAMERA_RIGHT, config.camera,
         config.fr_output,
         config.fr_input,
         config.fr_stat_interval
     );
}

static const char short_options[] = "m:hp:f:g:t:vi:o:s:k:c:";

static const struct option long_options[] = {
    { "mode",   required_argument, NULL, 'm' },
    { "help",   no_argument,       NULL, 'h' },
    { "preview",required_argument, NULL, 'p' },
    { "camera", required_argument, NULL, 'c' },
    { "ofr",    required_argument, NULL, 'f' },
    { "ifr",    required_argument, NULL, 'g' },
    { "sfr",    required_argument, NULL, 't' },
    { "verbose", no_argument,      NULL, 'v' },
    { "input",  required_argument, NULL, 'i' },
    { "output", required_argument, NULL, 'o' },
    { "save",   required_argument, NULL, 's' },
    { "skip",   required_argument, NULL, 'k' },
    { 0, 0, 0, 0 }
};

void parse_arg(int argc, char** argv)
{
    int camera;
    for (;;) {
        int idx;
        int c;

        c = getopt_long(argc, argv, short_options, long_options, &idx);

        if (-1 == c)
            break;

        switch (c) {
        case 0: /* getopt_long() flag */
            break;

        case 'h':
            usage(stdout, argc, argv);
            exit(EXIT_SUCCESS);
            break;

        case 'm':
            config.cap_mode = (enum CapMode)atoi(optarg);
            break;

        case 'c':
            camera = atoi(optarg);
            if (camera != CAMERA_ANY && camera != CAMERA_LEFT && camera != CAMERA_RIGHT)
            {
                log_error("invalid camera input\n");
                exit(EXIT_FAILURE);
            }
            config.camera = (enum CameraId) camera;
            break;

        case 'p':
            config.enable_preview = atoi(optarg) ? true : false;
            break;

        case 'f':
            config.fr_output = atoi(optarg);
            break;

        case 'g':
            config.fr_input = atoi(optarg);
            break;

        case 't':
            config.fr_stat_interval = atoi(optarg);
            break;

        case 's':
            g_save_frame_count = atoi(optarg);
            if (g_save_frame_count < 0)
            {
                log_error("The save frame number %d is not correct, it must be a positive number.\n", g_save_frame_count);
                exit(EXIT_FAILURE);
            }
            break;
        case 'k':
            g_skip_frame_count = atoi(optarg);
            if (g_skip_frame_count < 0)
            {
                log_error("The skip frame number %d is not correct, it must be a postive number\n", g_skip_frame_count);
                exit(EXIT_FAILURE);
            }
            break;

        case 'o':
            config.output_dev = (enum OutputDevice)atoi(optarg);
            break;

        case 'i':
            config.input_dev = (enum InputDevice)atoi(optarg);
            break;

        case 'v':
            log_set_level(LOG_DEBUG);
            break;

        default:
            usage(stderr, argc, argv);
            exit(EXIT_FAILURE);
        }
    }
}

static int do_isp_i2c(enum CameraId camera, int argc, char** argv)
{
    uint16_t regAddr;
    uint8_t  data;

    if (com_open() < 0)
    {
        log_debug("eim open fails\n");
        return -1;
    }

    if (argc <= 2 || argc > 5)
    {
        log_error("the length of i2c command is not correct!\n");
        goto err;
    }

    switch(argv[2][0])
    {
        case 't':
        case 'T':  //i2c t
            isp_test();
        break;

        case 'r':
        case 'R':  //i2c r <register>
            if (argc != 4) {
                log_error("the length for i2c read command is not correct\n");
                goto err;
            }
            regAddr = (uint16_t)strtol(argv[3], NULL, 16);
            if (isp_read(camera, regAddr, &data) < 0)
            {
                log_error("I2C read fails, Register=0x%04x", regAddr);
                goto err;
            }
            log_info("I2C read success. Register=0x%04X, Data=0x%04X\n", regAddr, data);
        break;

        case 'w':
        case 'W': //i2c w <register> <data>
            if (argc != 5) {
                log_error("the length for i2c write command is not correct (%d)\n", argc);
                goto err;
            }
            regAddr = (uint16_t)strtol(argv[3], NULL, 16);
            data = (uint8_t)strtol(argv[4], NULL, 16);
            if (isp_write(camera, regAddr, data) < 0)
            {
                log_error("I2C read fails, Register=0x%04x, Data=0x%02X", regAddr, data);
                goto err;
            }
            log_info("I2C write success. Register=0x%04X, Data=0x%02X\n", regAddr, data);

            //add delay to avoid reset the eim controller too earlier, as the FPGA is currently
            //working on writing data
            usleep(1000000);
        break;

        default:
            log_error("Unrecongize command %s\n", argv[2]);
            goto err;
    }

    com_close();
    return 0;
err:
    com_close();
    usage_i2c(stdout, argc, argv);
    return -1;
}

static int do_camera_i2c(enum CameraId camera, int argc, char** argv)
{
    uint16_t regAddr;
    uint8_t  data;

    if (com_open() < 0)
    {
        log_debug("eim open fails\n");
        return -1;
    }

    if (argc <= 2 || argc > 5)
    {
        log_error("the length of i2c command is not correct!\n");
        goto err;
    }

    switch(argv[2][0])
    {
        case 't':
        case 'T':  //i2c t
            camera_i2c_test();
        break;

        case 'r':
        case 'R':  //i2c r <register>
            if (argc != 4) {
                log_error("the length for i2c read command is not correct\n");
                goto err;
            }
            regAddr = (uint16_t)strtol(argv[3], NULL, 16);
            if (camera_read(camera, regAddr, &data) < 0)
            {
                log_error("Camera I2C read fails, Register=0x%04x", regAddr);
                goto err;
            }
            log_info("I2C read success. Register=0x%04X, Data=0x%04X\n", regAddr, data);
        break;

        case 'w':
        case 'W': //i2c w <register> <data>
            if (argc != 5) {
                log_error("the length for i2c write command is not correct (%d)\n", argc);
                goto err;
            }
            regAddr = (uint16_t)strtol(argv[3], NULL, 16);
            data = (uint8_t)strtol(argv[4], NULL, 16);
            if (camera_write(camera, regAddr, data) < 0)
            {
                log_error("Camera I2C read fails, Register=0x%04x, Data=0x%02X", regAddr, data);
                goto err;
            }
            log_info("Camera I2C write success. Register=0x%04X, Data=0x%02X\n", regAddr, data);

            //add delay to avoid reset the eim controller too earlier, as the FPGA is currently
            //working on writing data
            usleep(1000000);
        break;

        default:
            log_error("Unrecongize command %s\n", argv[2]);
            goto err;
    }

    com_close();
    return 0;
err:
    com_close();
    usage_i2c(stdout, argc, argv);
    return -1;
}

static int do_reg_test(int argc, char** argv)
{
    uint8_t regAddr;
    uint16_t  data;

    if (com_open() < 0)
    {
        log_debug("eim open fails\n");
        return -1;
    }

    if (argc <= 2 || argc > 5)
    {
        log_error("the length of register command is not correct!\n");
        goto err;
    }

    switch(argv[2][0])
    {
        case 't':
        case 'T':  //i2c t
            reg_test();
        break;

        case 'r':
        case 'R':  //reg r <register>
            if (argc != 4) {
                log_error("the length for reg read command is not correct\n");
                goto err;
            }
            regAddr = (uint8_t)strtol(argv[3], NULL, 16);
            data = reg_read(regAddr);
            log_info("register read success. Register=0x%04X, Data=0x%04X\n", regAddr, data);
        break;

        case 'w':
        case 'W': //reg w <register> <data>
            if (argc != 5) {
                log_error("the length for register write command is not correct\n");
                goto err;
            }
            regAddr = (uint8_t)strtol(argv[3], NULL, 16);
            data = (uint16_t)strtol(argv[4], NULL, 16);
            reg_write(regAddr, data);
            log_info("reg write success. Register=0x%02X, Data=0x%04X\n", regAddr, data);
        break;

        default:
            log_error("Unrecongize command %s\n", argv[2]);
            goto err;
    }

    com_close();
    return 0;
err:
    com_close();
    usage_reg(stdout, argc, argv);
    return -1;

}

static void signal_handler(int sig)
{
    log_info("Receive signal %d\n", sig);
    capmgr_stop();
}

void *thread_capmgr(void* arg)
{
    log_info("capmgr thread start\n");
    capmgr_mainloop();

    log_info("capmgr mainloop exits\n");

    capmgr_close();
    return NULL;
}

void *thread_control(void* arg)
{
    log_info("control thread start\n");

    int c;
    while(1)
    {
        c = getchar();
        switch(c)
        {
            case 's':
                log_info("Start to save a frame...\n");
                capmgr_save(1, 0, NULL);
                break;
            case 'a':
                capmgr_set_camera(CAMERA_LEFT);
                break;
            case 'b':
                capmgr_set_camera(CAMERA_RIGHT);
                break;
            break;

        }
        usleep(5000);
    }
}

int main(int argc, char** argv)
{
    pthread_t capmgr_thread_id, ctrl_thread_id;
    int ret;

    if (argc > 1 && strcmp(argv[1], "reg") == 0)
    {
        return do_reg_test(argc, argv);
    }
    else if (argc > 1 && strcmp(argv[1], "isp") == 0)
    {
        if (do_isp_i2c(CAMERA_LEFT, argc, argv) < 0) return -1;
        return do_isp_i2c(CAMERA_RIGHT, argc, argv);
    }
    else if (argc > 1 && strcmp(argv[1], "isp0") == 0)
    {
        return do_isp_i2c(CAMERA_LEFT, argc, argv);
    }
    else if (argc > 1 && strcmp(argv[1], "isp1") == 0)
    {
        return do_isp_i2c(CAMERA_RIGHT, argc, argv);
    }
    else if (argc > 1 && strcmp(argv[1], "camera") == 0)
    {
        if (do_camera_i2c(CAMERA_LEFT, argc, argv) < 0) return -1;
        return do_camera_i2c(CAMERA_RIGHT, argc, argv);
    }
    else if (argc > 1 && strcmp(argv[1], "camera0") == 0)
    {
        return do_camera_i2c(CAMERA_LEFT, argc, argv);
    }
    else if (argc > 1 && strcmp(argv[1], "camera1") == 0)
    {
        return do_camera_i2c(CAMERA_RIGHT, argc, argv);
    }

    log_set_level(LOG_DEBUG);
    config.cap_mode = CAP_MODE_CONTOUR;
    config.fr_stat_interval = 5000;
    config.enable_preview =  1;
    config.fr_output = 60;
    config.fr_input = 60;
    config.input_dev = DEV_INPUT_FPGA;
    config.output_dev = DEV_OUTPUT_FB;
    config.camera = CAMERA_ANY;
    g_save_frame_count = 0;
    g_skip_frame_count = 0;

    parse_arg(argc, argv);
    workaround();

    //signal(SIGINT, signal_handler);

    if (capmgr_open(&config) < 0)
    {
        return 1;
    }

    if (g_save_frame_count > 0)
    {
        log_info("Receive request to save %d frames!\n", g_save_frame_count);
        capmgr_save(g_save_frame_count, g_skip_frame_count, NULL);
    }

    ret = pthread_create(&ctrl_thread_id, NULL, thread_control, NULL);
    if (ret != 0)
    {
        log_error("Create dump fifo thread fails, ret=%d\n", ret);
        return 1;
    }

    ret = pthread_create(&capmgr_thread_id, NULL, thread_capmgr, NULL);
    if (ret != 0)
    {
        log_error("Create Capture Manager thread fails, ret=%d\n", ret);
        return 1;
    }

    ret = pthread_join(capmgr_thread_id, NULL);
    if (ret != 0)
    {
        log_error("Error joining capmgr thread: %d\n", ret);
        return 1;
    }

    ret = pthread_join(ctrl_thread_id, NULL);
    if (ret != 0)
    {
        log_error("Error joining control thread: %d\n", ret);
        return 1;
    }
    return 0;
}
