#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "common.h"
#include "contour.h"
#include "capture_manager.h"
#include "contour_fifo.h"

static int init()
{
    capmgr_config_t config;

    log_set_level(LOG_DEBUG);

    config.cap_mode = CAP_MODE_CONTOUR;
    config.fr_stat_interval = 10000;
    config.enable_preview =  1;
    config.fr_output = 60;
    config.fr_input = 60;
    config.input_dev = DEV_INPUT_FPGA;
    config.output_dev = DEV_OUTPUT_FIFO;

    return capmgr_open(&config);
}

void *thread_capmgr(void* arg)
{
    capmgr_mainloop();

    log_info("capmgr mainloop exits");

    capmgr_close();
    return NULL;
}

void *thread_dump_fifo(void* arg)
{
    frame_contours_t *fc = NULL;
    fifo_handle_t fifo = global_fifo_get();
    if (fifo == NULL)
    {
        log_error("fifo is not initialized!\n");
        exit(1);
    }

    while(true)
    {
        fc = cfifo_pop(fifo);
        if (fc == NULL) //FIFO is empty, wait a while
        {
            usleep(1000);
            continue;
        }
        log_info("CAM[0]: NumContours: %d, NumPointsOfFirstContour: %d\n",
            fc->cameras[0].num_of_contours,
            fc->cameras[0].contours[0].count);
        log_info("CAM[1]: NumContours: %d, NumPointsOfFirstContour: %d\n",
            fc->cameras[1].num_of_contours,
            fc->cameras[1].contours[1].count);
    }
}

int main(int argc, char** argv)
{
    pthread_t capmgr_id, dump_id;
    int ret;

    workaround();

    if (init() < 0)
    {
        log_error("init fails\n");
        exit(1);
    }

    ret = pthread_create(&dump_id, NULL, thread_dump_fifo, NULL);
    if (ret != 0)
    {
        log_error("Create dump fifo thread fails, ret=%d\n", ret);
        exit(1);
    }

    ret = pthread_create(&capmgr_id, NULL, thread_capmgr, NULL);
    if (ret != 0)
    {
        log_error("Create Capture Manager thread fails, ret=%d\n", ret);
        exit(1);
    }

    ret = pthread_join(capmgr_id, NULL);
    if (ret != 0)
    {
        log_error("Error joining capmgr thread: %d", ret);
        exit(1);
    }

    ret = pthread_join(dump_id, NULL);
    if (ret != 0)
    {
        log_error("Error joining dump fifo thread: %d", ret);
        exit(1);
    }

    return 0;
}
