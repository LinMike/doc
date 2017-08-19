/*
 * Copyright 2004-2013 Freescale Semiconductor, Inc. All rights reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*
 * @file mxc_v4l2_overlay.c
 *
 * @brief Mxc Video For Linux 2 driver test application
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/*=======================================================================
                                        INCLUDE FILES
=======================================================================*/
/* Standard Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <sys/time.h>

#include <linux/mxcfb.h>
#include <linux/mxc_v4l2.h>
#include <linux/ipu.h>

#include "output.h"
#include "../common.h"

#define TFAIL -1
#define TPASS 0

int fd_v4l = 0;
int fd_ipu = 0;
int g_frame_size;
int g_num_buffers;

char* g_dev = "/dev/video18";
int g_overlay = 0;
enum v4l2_memory g_mem_type = V4L2_MEMORY_MMAP;
int g_in_width = 1280;
int g_in_height = 720;
int g_display_width = 1920;
int g_display_height = 1080;
int g_display_top = 0;
int g_display_left = 0;
int g_rotate = 0;
int g_vflip = 0;
int g_hflip = 0;
int g_vdi_enable = 0;
int g_vdi_motion = 0;
int g_icrop_w = 0;
int g_icrop_h = 0;
int g_icrop_top = 0;
int g_icrop_left = 0;
uint32_t g_in_fmt = V4L2_PIX_FMT_YUV420;

int g_bmp_header = 0;
int g_loop_count = 1;
int g_frame_period = 33333;

#define MAX_BUFFER_NUM 16

struct testbuffer
{
    unsigned char* start;
    size_t offset;
    unsigned int length;
};

struct testbuffer buffers[MAX_BUFFER_NUM];

void fb_setup(void)
{
    struct mxcfb_gbl_alpha alpha;
    int fd;

#ifdef BUILD_FOR_ANDROID
    fd = open("/dev/graphics/fb0", O_RDWR);
#else
    fd = open("/dev/fb0", O_RDWR);
#endif

    alpha.alpha = 0;
    alpha.enable = 1;
    if (ioctl(fd, MXCFB_SET_GBL_ALPHA, &alpha) < 0)
    {
        printf("set alpha %d failed for fb0\n", alpha.alpha);
    }

    close(fd);
}

static int init_buf()
{
    int i;
    int err = 0;
    int retval = 0;
    int type;
    struct v4l2_buffer buf;
    __u32 total_time;
    int count = 100;
    int streamon_cnt = 0;
    struct timeval tv_start, tv_current;

    if (g_vdi_enable)
        streamon_cnt = 1;

    memset(&buf, 0, sizeof(buf));

    if (g_mem_type == V4L2_MEMORY_MMAP)
    {
        for (i = 0; i < g_num_buffers; i++)
        {
            memset(&buf, 0, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
            buf.memory = g_mem_type;
            buf.index = i;
            if (ioctl(fd_v4l, VIDIOC_QUERYBUF, &buf) < 0)
            {
                printf("VIDIOC_QUERYBUF error\n");
                retval = -1;
                goto cleanup;
            }

            buffers[i].length = buf.length;
            buffers[i].offset = (size_t)buf.m.offset;
            printf("VIDIOC_QUERYBUF: length = %d, offset = %d\n", buffers[i].length,
                buffers[i].offset);
            buffers[i].start = (unsigned char*)mmap(NULL, buffers[i].length, PROT_READ | PROT_WRITE,
                MAP_SHARED, fd_v4l, buffers[i].offset);
            if (buffers[i].start == NULL)
            {
                printf("v4l2_out test: mmap failed\n");
                retval = -1;
                goto cleanup;
            }
        }
    }

    gettimeofday(&tv_start, 0);
    printf("start time = %d s, %d us\n", tv_start.tv_sec, tv_start.tv_usec);

    return 0;

cleanup:
    if (g_mem_type == V4L2_MEMORY_MMAP)
    {
        for (i = 0; i < g_num_buffers; i++)
        {
            munmap(buffers[i].start, buffers[i].length);
        }
    }
    return retval;
}


static int hdmi_write_frame(unsigned char * frame_buf, int frame_size)
{
    static int i = 0;
    int err = 0;
    int retval = 0;
    int type;
    struct v4l2_buffer buf;
    int count = 100;
    int streamon_cnt = 0;
    struct timeval tv_start, tv_current;

    if (g_vdi_enable)
        streamon_cnt = 1;

    memset(&buf, 0, sizeof(buf));

    buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf.memory = g_mem_type;
    if (i < g_num_buffers)
    {
        buf.index = i;
        if (ioctl(fd_v4l, VIDIOC_QUERYBUF, &buf) < 0)
        {
            printf("VIDIOC_QUERYBUF failed\n");
            retval = -1;
            goto err;
        }
        if (g_mem_type == V4L2_MEMORY_USERPTR)
        {
            buf.m.userptr = (unsigned long)buffers[i].offset;
            buf.length = buffers[i].length;
        }
    }
    else
    {
        buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        buf.memory = g_mem_type;
        if (ioctl(fd_v4l, VIDIOC_DQBUF, &buf) < 0)
        {
            printf("VIDIOC_DQBUF failed\n");
            retval = -1;
            goto err;
        }
    }

    memcpy(buffers[buf.index].start, frame_buf, frame_size);

    buf.timestamp.tv_sec = tv_start.tv_sec;
    buf.timestamp.tv_usec = tv_start.tv_usec + (g_frame_period * i);
    if (g_vdi_enable)
        buf.field = V4L2_FIELD_INTERLACED_TB;
    if ((retval = ioctl(fd_v4l, VIDIOC_QBUF, &buf)) < 0)
    {
        printf("VIDIOC_QBUF failed %d\n", retval);
        retval = -1;
        goto err;
    }

    if (i == streamon_cnt)
    { // Start playback after buffers queued
        type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        if (ioctl(fd_v4l, VIDIOC_STREAMON, &type) < 0)
        {
            printf("Could not start stream\n");
            retval = -1;
            goto err;
        }
        /*simply set fb0 alpha to 0*/
        fb_setup();
    }

    return 0;

err:
    return retval;
}

int mxc_v4l_output_setup(struct v4l2_format* fmt)
{
    struct v4l2_requestbuffers buf_req;

    if (ioctl(fd_v4l, VIDIOC_S_FMT, fmt) < 0)
    {
        printf("set format failed\n");
        return TFAIL;
    }

    if (ioctl(fd_v4l, VIDIOC_G_FMT, fmt) < 0)
    {
        printf("get format failed\n");
        return TFAIL;
    }

    memset(&buf_req, 0, sizeof(buf_req));
    buf_req.count = 4;
    buf_req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf_req.memory = g_mem_type;
    if (ioctl(fd_v4l, VIDIOC_REQBUFS, &buf_req) < 0)
    {
        printf("request buffers failed\n");
        return TFAIL;
    }
    g_num_buffers = buf_req.count;
    printf("v4l2_output test: Allocated %d buffers\n", buf_req.count);

    return TPASS;
}

void memfree(int buf_size, int buf_cnt)
{
    int i;
    unsigned int page_size;

    page_size = getpagesize();
    buf_size = (buf_size + page_size - 1) & ~(page_size - 1);

    for (i = 0; i < buf_cnt; i++)
    {
        if (buffers[i].start)
            munmap(buffers[i].start, buf_size);
        if (buffers[i].offset)
            ioctl(fd_ipu, IPU_FREE, &buffers[i].offset);
    }
    close(fd_ipu);
}

int memalloc(int buf_size, int buf_cnt)
{
    int i, ret = TPASS;
    unsigned int page_size;

    fd_ipu = open("/dev/mxc_ipu", O_RDWR, 0);
    if (fd_ipu < 0)
    {
        printf("open ipu dev fail\n");
        return TFAIL;
    }

    for (i = 0; i < buf_cnt; i++)
    {
        page_size = getpagesize();
        buf_size = (buf_size + page_size - 1) & ~(page_size - 1);
        buffers[i].length = buffers[i].offset = buf_size;
        ret = ioctl(fd_ipu, IPU_ALLOC, &buffers[i].offset);
        if (ret < 0)
        {
            printf("ioctl IPU_ALLOC fail\n");
            ret = TFAIL;
            goto err;
        }
        buffers[i].start = (unsigned char*)mmap(
            0, buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_ipu, buffers[i].offset);
        if (!buffers[i].start)
        {
            printf("mmap fail\n");
            ret = TFAIL;
            goto err;
        }
        printf("USRP: alloc bufs offset 0x%x size %d\n", buffers[i].offset, buf_size);
    }

    return ret;
err:
    memfree(buf_size, buf_cnt);
    return ret;
}

static int hdmi_open(output_param_t* param)
{
    return 0;
}

static int hdmi_start()
{
    struct v4l2_control ctrl;
    struct v4l2_format fmt;
    struct v4l2_framebuffer fb;
    struct v4l2_cropcap cropcap;
    struct v4l2_capability cap;
    struct v4l2_fmtdesc fmtdesc;
    struct v4l2_crop crop;
    struct v4l2_rect icrop;
    int retval = TPASS;

    if ((fd_v4l = open(g_dev, O_RDWR, 0)) < 0)
    {
        printf("Unable to open %s\n", g_dev);
        retval = TFAIL;
        goto err0;
    }

    if (!ioctl(fd_v4l, VIDIOC_QUERYCAP, &cap))
    {
        printf("driver=%s, card=%s, bus=%s, "
               "version=0x%08x, "
               "capabilities=0x%08x\n",
            cap.driver, cap.card, cap.bus_info, cap.version, cap.capabilities);
    }

    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    while (!ioctl(fd_v4l, VIDIOC_ENUM_FMT, &fmtdesc))
    {
        printf("fmt %s: fourcc = 0x%08x\n", fmtdesc.description, fmtdesc.pixelformat);
        fmtdesc.index++;
    }

    memset(&cropcap, 0, sizeof(cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    if (ioctl(fd_v4l, VIDIOC_CROPCAP, &cropcap) < 0)
    {
        printf("get crop capability failed\n");
        retval = TFAIL;
        goto err1;
    }
    printf("cropcap.bounds.width = %d\ncropcap.bound.height = %d\n"
           "cropcap.defrect.width = %d\ncropcap.defrect.height = %d\n",
        cropcap.bounds.width, cropcap.bounds.height, cropcap.defrect.width, cropcap.defrect.height);

    crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    crop.c.top = g_display_top;
    crop.c.left = g_display_left;
    crop.c.width = g_display_width;
    crop.c.height = g_display_height;
    if (ioctl(fd_v4l, VIDIOC_S_CROP, &crop) < 0)
    {
        printf("set crop failed\n");
        retval = TFAIL;
        goto err1;
    }

    // Set rotation
    ctrl.id = V4L2_CID_ROTATE;
    ctrl.value = g_rotate;
    if (ioctl(fd_v4l, VIDIOC_S_CTRL, &ctrl) < 0)
    {
        printf("set ctrl rotate failed\n");
        retval = TFAIL;
        goto err1;
    }
    ctrl.id = V4L2_CID_VFLIP;
    ctrl.value = g_vflip;
    if (ioctl(fd_v4l, VIDIOC_S_CTRL, &ctrl) < 0)
    {
        printf("set ctrl vflip failed\n");
        retval = TFAIL;
        goto err1;
    }
    ctrl.id = V4L2_CID_HFLIP;
    ctrl.value = g_hflip;
    if (ioctl(fd_v4l, VIDIOC_S_CTRL, &ctrl) < 0)
    {
        printf("set ctrl hflip failed\n");
        retval = TFAIL;
        goto err1;
    }
    if (g_vdi_enable)
    {
        ctrl.id = V4L2_CID_MXC_MOTION;
        ctrl.value = g_vdi_motion;
        if (ioctl(fd_v4l, VIDIOC_S_CTRL, &ctrl) < 0)
        {
            printf("set ctrl motion failed\n");
            retval = TFAIL;
            goto err1;
        }
    }

    // Set framebuffer parameter for MX27
    fb.capability = V4L2_FBUF_CAP_EXTERNOVERLAY;
    if (g_overlay)
        fb.flags = V4L2_FBUF_FLAG_OVERLAY;
    else
        fb.flags = V4L2_FBUF_FLAG_PRIMARY;
    ioctl(fd_v4l, VIDIOC_S_FBUF, &fb);

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width = g_in_width;
    fmt.fmt.pix.height = g_in_height;
    fmt.fmt.pix.pixelformat = g_in_fmt;
    if (g_vdi_enable)
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED_TB;
    if (g_icrop_w && g_icrop_h)
    {
        icrop.left = g_icrop_left;
        icrop.top = g_icrop_top;
        icrop.width = g_icrop_w;
        icrop.height = g_icrop_h;
        fmt.fmt.pix.priv = (unsigned int)&icrop;
    }
    else
        fmt.fmt.pix.priv = 0;
    retval = mxc_v4l_output_setup(&fmt);
    if (retval < 0)
        goto err1;

    g_frame_size = fmt.fmt.pix.sizeimage;

    if (g_mem_type == V4L2_MEMORY_USERPTR)
        if (memalloc(g_frame_size, g_num_buffers) < 0)
            goto err1;

    retval = init_buf();
    if (retval < 0)
        goto err1;

    return 0;

err1:
    close(fd_v4l);
err0:
    return retval;
}


static void hdmi_close()
{
    int i;
    int type;
    type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ioctl(fd_v4l, VIDIOC_STREAMOFF, &type);

    if (g_mem_type == V4L2_MEMORY_MMAP)
    {
        for (i = 0; i < g_num_buffers; i++)
        {
            munmap(buffers[i].start, buffers[i].length);
        }
    }
    else if (g_mem_type == V4L2_MEMORY_USERPTR)
    {
        memfree(g_frame_size, g_num_buffers);
    }
    close(fd_v4l);
    fd_v4l = 0;
}

output_device_t hdmi_v4l2_output_device = {
    .name = "HDMI v4l2 output",
    .open = hdmi_open,
    .start = hdmi_start,
    .close = hdmi_close,
    .write_frame = hdmi_write_frame,
    .get_next_buf = NULL,
};

#ifdef __cplusplus
}
#endif
