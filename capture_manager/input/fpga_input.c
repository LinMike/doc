/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *  This program were got from V4L2 API, Draft 0.20
 *      available at: http://v4l2spec.bytesex.org/
 */

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h> /* getopt_long() */

#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include "input.h"


//#define FORCED_FORMAT V4L2_PIX_FMT_YUV420
#define FORCED_FIELD V4L2_FIELD_ANY

enum io_method
{
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
};

struct buffer
{
    void* start;
    size_t length;
};

static input_param_t g_cap_param;
static int g_fd = -1;
static enum io_method io = IO_METHOD_MMAP;
static struct buffer* buffers;
static unsigned int n_buffers;

static void errno_dump(const char* s)
{
    log_error( "%s error %d, %s\n", s, errno, strerror(errno));
}

static int xioctl(int fh, int request, void* arg)
{
    int r;

    do
    {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

static int fpga_read_frame(unsigned char** frame_buf, int* frame_size)
{
    struct v4l2_buffer buf;
    unsigned int i;

    //log_debug("%s: called!\n", __func__);

    switch (io)
    {
        case IO_METHOD_READ:
            if (-1 == read(g_fd, buffers[0].start, buffers[0].length))
            {
                switch (errno)
                {
                    case EAGAIN:
                        return 0;

                    case EIO:
                    /* Could ignore EIO, see spec. */

                    /* fall through */

                    default:
                        errno_dump("read");
                        return -1;
                }
            }

            *frame_buf = (unsigned char*)buffers[0].start;
            *frame_size = buffers[0].length;

            //process_image(buffers[0].start, buffers[0].length);
            break;

        case IO_METHOD_MMAP:
            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            if (-1 == xioctl(g_fd, VIDIOC_DQBUF, &buf))
            {
                switch (errno)
                {
                    case EAGAIN:
                        return 0;

                    case EIO:
                    /* Could ignore EIO, see spec. */

                    /* fall through */

                    default:
                        errno_dump("VIDIOC_DQBUF");
                        return -1;
                }
            }

            assert(buf.index < n_buffers);

            *frame_buf = (unsigned char*)buffers[buf.index].start;
            *frame_size = buf.bytesused;

            //process_image(buffers[buf.index].start, buf.bytesused);

            if (-1 == xioctl(g_fd, VIDIOC_QBUF, &buf)) {
                errno_dump("VIDIOC_QBUF");
                return -1;
            }
            break;

        case IO_METHOD_USERPTR:
            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;

            if (-1 == xioctl(g_fd, VIDIOC_DQBUF, &buf))
            {
                switch (errno)
                {
                    case EAGAIN:
                        return 0;

                    case EIO:
                    /* Could ignore EIO, see spec. */

                    /* fall through */

                    default:
                        errno_dump("VIDIOC_DQBUF");
                        return -1;
                }
            }

            for (i = 0; i < n_buffers; ++i)
                if (buf.m.userptr == (unsigned long)buffers[i].start
                    && buf.length == buffers[i].length)
                    break;

            assert(i < n_buffers);

            *frame_buf = (unsigned char*)buf.m.userptr;
            *frame_size = buf.bytesused;

            if (-1 == xioctl(g_fd, VIDIOC_QBUF, &buf)) {
                errno_dump("VIDIOC_QBUF");
                return -1;
            }
            break;
    }

    return 1;
}


static int stop_capturing(void)
{
    enum v4l2_buf_type type;

    log_debug("%s: called!\n", __func__);

    switch (io)
    {
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(g_fd, VIDIOC_STREAMOFF, &type)) {
                errno_dump("VIDIOC_STREAMOFF");
                return -1;
            }
            break;
    }
}

static int start_capturing(void)
{
    unsigned int i;
    enum v4l2_buf_type type;
    int err;

    log_debug("%s: called!\n", __func__);

    //log_debug("\tn_buffers: %d\n", n_buffers);

    switch (io)
    {
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;

        case IO_METHOD_MMAP:
            for (i = 0; i < n_buffers; ++i)
            {
                struct v4l2_buffer buf;

                //log_debug("\ti: %d\n", i);

                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                //log_debug("\tbuf.index: %d\n", buf.index);

                err == xioctl(g_fd, VIDIOC_QBUF, &buf);
                //log_debug("\terr: %d\n", err);

                if (-1 == err) {
                    errno_dump("VIDIOC_QBUF");
                    return -1;
                }

                log_debug("\tbuffer queued!\n");
            }

            //log_debug("Before STREAMON\n");
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(g_fd, VIDIOC_STREAMON, &type)) {
                errno_dump("VIDIOC_STREAMON");
                return -1;
            }
            //log_debug("After STREAMON\n");
            break;

        case IO_METHOD_USERPTR:
            for (i = 0; i < n_buffers; ++i)
            {
                struct v4l2_buffer buf;

                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_USERPTR;
                buf.index = i;
                buf.m.userptr = (unsigned long)buffers[i].start;
                buf.length = buffers[i].length;

                if (-1 == xioctl(g_fd, VIDIOC_QBUF, &buf)) {
                    errno_dump("VIDIOC_QBUF");
                    return -1;
                }
            }
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(g_fd, VIDIOC_STREAMON, &type)) {
                errno_dump("VIDIOC_STREAMON");
                return -1;
            }
            break;
    }

    return 0;
}

static int uninit_device(void)
{
    unsigned int i;

    log_debug("%s: called!\n", __func__);

    switch (io)
    {
        case IO_METHOD_READ:
            FREE(buffers[0].start);
            break;

        case IO_METHOD_MMAP:
            for (i = 0; i < n_buffers; ++i)
                if (-1 == munmap(buffers[i].start, buffers[i].length)) {
                    errno_dump("munmap");
                    return -1;
                }
            break;

        case IO_METHOD_USERPTR:
            for (i = 0; i < n_buffers; ++i)
                FREE(buffers[i].start);
            break;
    }

    FREE(buffers);
}

static int init_read(unsigned int buffer_size)
{
    log_debug("%s: called!\n", __func__);

    buffers = (struct buffer*)calloc(1, sizeof(*buffers));

    if (!buffers)
    {
        log_error( "Out of memory\n");
        return -1;
    }

    buffers[0].length = buffer_size;
    buffers[0].start = malloc(buffer_size);

    if (!buffers[0].start)
    {
        log_error( "Out of memory\n");
        FREE(buffers);
        return -1;
    }
    return 0;
}

static int init_mmap(void)
{
    struct v4l2_requestbuffers req;

    log_debug("%s: called!\n", __func__);

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(g_fd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            log_error( "%s does not support "
                            "memory mapping\n",
                g_cap_param.dev);
            return -1;
        }
        else
        {
            errno_dump("VIDIOC_REQBUFS");
            return -1;
        }
    }
    log_debug("\treq.count: %d\n", req.count);
    log_debug("\treq.type: %d\n", req.type);
    log_debug("\treq.memory: %d\n", req.memory);
    log_debug("\n");


    if (req.count < 2)
    {
        log_error( "Insufficient buffer memory on %s\n", g_cap_param.dev);
        return -1;
    }

    buffers = (struct buffer*)calloc(req.count, sizeof(*buffers));

    if (!buffers)
    {
        log_error( "Out of memory\n");
        return -1;
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (-1 == xioctl(g_fd, VIDIOC_QUERYBUF, &buf)) {
            errno_dump("VIDIOC_QUERYBUF");
            return -1;
        }
        /*
        log_debug("\tbuf.index: %d\n", buf.index);
        log_debug("\tbuf.type: %d\n", buf.type);
        log_debug("\tbuf.bytesused: %d\n", buf.bytesused);
        log_debug("\tbuf.flags: %d\n", buf.flags);
        log_debug("\tbuf.field: %d\n", buf.field);
        log_debug("\tbuf.timestamp.tv_sec: %ld\n", (long)buf.timestamp.tv_sec);
        log_debug("\tbuf.timestamp.tv_usec: %ld\n", (long)buf.timestamp.tv_usec);
        log_debug("\tbuf.timecode.type: %d\n", buf.timecode.type);
        log_debug("\tbuf.timecode.flags: %d\n", buf.timecode.flags);
        log_debug("\tbuf.timecode.frames: %d\n", buf.timecode.frames);
        log_debug("\tbuf.timecode.seconds: %d\n", buf.timecode.seconds);
        log_debug("\tbuf.timecode.minutes: %d\n", buf.timecode.minutes);
        log_debug("\tbuf.timecode.hours: %d\n", buf.timecode.hours);
        log_debug("\tbuf.timecode.userbits: %d,%d,%d,%d\n", buf.timecode.userbits[0],
            buf.timecode.userbits[1], buf.timecode.userbits[2], buf.timecode.userbits[3]);
        log_debug("\tbuf.sequence: %d\n", buf.sequence);
        log_debug("\tbuf.memory: %d\n", buf.memory);
        log_debug("\tbuf.m.offset: %d\n", buf.m.offset);
        log_debug("\tbuf.length: %d\n", buf.length);
        log_debug("\tbuf.input: %d\n", buf.input);
        log_debug("\n");
        */
        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start = mmap(NULL /* start anywhere */, buf.length,
            PROT_READ | PROT_WRITE /* required */, MAP_SHARED /* recommended */, g_fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start) {
            errno_dump("mmap");
            return -1;
        }
    }
}

static int init_userp(unsigned int buffer_size)
{
    struct v4l2_requestbuffers req;

    log_debug("%s: called!\n", __func__);

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (-1 == xioctl(g_fd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            log_error( "%s does not support "
                            "user pointer i/o\n",
                g_cap_param.dev);
            return -1;
        }
        else
        {
            errno_dump("VIDIOC_REQBUFS");
            return -1;
        }
    }

    buffers = (struct buffer*)calloc(4, sizeof(*buffers));

    if (!buffers)
    {
        log_error( "Out of memory\n");
        return -1;
    }

    for (n_buffers = 0; n_buffers < 4; ++n_buffers)
    {
        buffers[n_buffers].length = buffer_size;
        buffers[n_buffers].start = malloc(buffer_size);

        if (!buffers[n_buffers].start)
        {
            log_error( "Out of memory\n");
            return -1;
        }
    }
}

static int init_device(void)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    struct v4l2_streamparm stream_param;
    unsigned int min;
    int input;

    if (-1 == xioctl(g_fd, VIDIOC_QUERYCAP, &cap))
    {
        if (EINVAL == errno)
        {
            log_error( "%s is no V4L2 device\n", g_cap_param.dev);
            return -1;
        }
        else
        {
            errno_dump("VIDIOC_QUERYCAP");
            return -1;
        }
    }

    log_debug("\tdriver: %s\n"
             "\tcard: %s \n"
             "\tbus_info: %s\n",
        cap.driver, cap.card, cap.bus_info);
    log_debug("\tversion: %u.%u.%u\n", (cap.version >> 16) & 0xFF, (cap.version >> 8) & 0xFF,
        cap.version & 0xFF);
    log_debug("\tcapabilities: 0x%08x\n", cap.capabilities);

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        log_error( "%s is no video capture device\n", g_cap_param.dev);
        return -1;
    }

    switch (io)
    {
        case IO_METHOD_READ:
            if (!(cap.capabilities & V4L2_CAP_READWRITE))
            {
                log_error( "%s does not support read i/o\n", g_cap_param.dev);
                return -1;
            }
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            if (!(cap.capabilities & V4L2_CAP_STREAMING))
            {
                log_error( "%s does not support streaming i/o\n", g_cap_param.dev);
                return -1;
            }
            break;
    }

    /*
    input = 1;
    if (xioctl(g_fd, VIDIOC_S_INPUT, &input) < 0)
    {
        log_error("VIDIOC_S_INPUT failed\n");
        return -1;
    }*/

    CLEAR(stream_param);
    stream_param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    stream_param.parm.capture.timeperframe.numerator = 1;
    stream_param.parm.capture.timeperframe.denominator = g_cap_param.framerate;
    stream_param.parm.capture.capturemode = g_cap_param.camera_mode;
    if (xioctl(g_fd, VIDIOC_S_PARM, &stream_param) < 0)
    {
        log_error( "%s does not support streaming i/o\n", g_cap_param.dev);
        return -1;
    }

    /* Select video input, video standard and tune here. */


    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(g_fd, VIDIOC_CROPCAP, &cropcap))
    {
        log_debug("\tcropcap.type: %d\n", cropcap.type);
        log_debug("\tcropcap.bounds.left: %d\n", cropcap.bounds.left);
        log_debug("\tcropcap.bounds.top: %d\n", cropcap.bounds.top);
        log_debug("\tcropcap.bounds.width: %d\n", cropcap.bounds.width);
        log_debug("\tcropcap.bounds.height: %d\n", cropcap.bounds.height);

        log_debug("\tcropcap.defrect.left: %d\n", cropcap.defrect.left);
        log_debug("\tcropcap.defrect.top: %d\n", cropcap.defrect.top);
        log_debug("\tcropcap.defrect.width: %d\n", cropcap.defrect.width);
        log_debug("\tcropcap.defrect.height: %d\n", cropcap.defrect.height);

        log_debug("\tcropcap.pixelaspect.numerator: %d\n", cropcap.pixelaspect.numerator);
        log_debug("\tcropcap.pixelaspect.denominator: %d\n", cropcap.pixelaspect.denominator);
        log_debug("\n");

        CLEAR(crop);
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl(g_fd, VIDIOC_S_CROP, &crop))
        {
            switch (errno)
            {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
                    log_debug("\tcropping not supported\n");
                    break;
            }
        }
    }
    else
    {
        /* Errors ignored. */
    }


    /* Set Format */
    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = g_cap_param.width;
    fmt.fmt.pix.height = g_cap_param.height;
    fmt.fmt.pix.pixelformat = g_cap_param.pixfmt;
    fmt.fmt.pix.field = FORCED_FIELD;

    log_debug("\nVIDIOC_S_FMT: resolution=%dx%d, pixfmt=%c%c%c%c, imagesize=%d, bytesperline=%d\n",
        fmt.fmt.pix.width,
        fmt.fmt.pix.height,
        fmt.fmt.pix.pixelformat & 0xFF,
            (fmt.fmt.pix.pixelformat >> 8 ) & 0xFF,
            (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
            (fmt.fmt.pix.pixelformat >> 24) & 0xFF,
        fmt.fmt.pix.sizeimage,
        fmt.fmt.pix.bytesperline);

    if (-1 == xioctl(g_fd, VIDIOC_S_FMT, &fmt)) {
        errno_dump("VIDIOC_S_FMT");
        return -1;
    }

    /* Dump Format Setting */
    if (-1 == xioctl(g_fd, VIDIOC_G_FMT, &fmt)) {
        errno_dump("VIDIOC_G_FMT");
        return -1;
    }

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    log_debug("VIDIOC_G_FMT: resolution=%dx%d, pixfmt=%c%c%c%c, imagesize=%d, bytesperline=%d\n",
        fmt.fmt.pix.width,
        fmt.fmt.pix.height,
        fmt.fmt.pix.pixelformat & 0xFF,
            (fmt.fmt.pix.pixelformat >> 8 ) & 0xFF,
            (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
            (fmt.fmt.pix.pixelformat >> 24) & 0xFF,
        fmt.fmt.pix.sizeimage,
        fmt.fmt.pix.bytesperline);

    switch (io)
    {
        case IO_METHOD_READ:
            init_read(fmt.fmt.pix.sizeimage);
            break;

        case IO_METHOD_MMAP:
            init_mmap();
            break;

        case IO_METHOD_USERPTR:
            init_userp(fmt.fmt.pix.sizeimage);
            break;
    }
}

static int close_device(void)
{
    log_debug("%s: called!\n", __func__);

    if (-1 == close(g_fd)) {
        errno_dump("close");
        return -1;
    }

    g_fd = -1;
}

static int open_device(const char* dev_name)
{
    struct stat st;
    int fd;

    log_debug("%s: called!\n", __func__);

    if (-1 == stat(dev_name, &st))
    {
        log_error("Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
        return -1;
    }

    if (!S_ISCHR(st.st_mode))
    {
        log_error("%s is no device\n", dev_name);
        return -1;
    }

    fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (-1 == fd)
    {
        log_error("Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
        return -1;
    }

    return fd;
}

static int fpga_open(input_param_t* param)
{
    log_debug("%s called\n", __func__);
    int fd;

    memcpy(&g_cap_param, param, sizeof(input_param_t));

    fd = open_device(g_cap_param.dev);
    if (fd <= 0)
    {
        return -1;
    }
    g_fd = fd;

    if (init_device() < 0)
    {
        log_error("%s: call init_device failed!\n", __func__);
        return -1;
    }

    return fd;
}

static int fpga_start()
{
    log_debug("%s called\n", __func__);
    if (start_capturing() < 0)
    {
        log_error("start capturing failed!\n");
        return -1;
    }

    return 0;
}

static void fpga_close()
{
    log_debug("%s called\n", __func__);
    stop_capturing();
    uninit_device();
    close_device();
    g_fd = -1;
}

static int fpga_get_fd()
{
    return g_fd;
}

input_device_t fpga_input_device = {
    .name = "fpga input",
    .open = fpga_open,
    .close = fpga_close,
    .start = fpga_start,
    .read_frame = fpga_read_frame,
    .get_fd = fpga_get_fd
};

#ifdef __cplusplus
}
#endif

