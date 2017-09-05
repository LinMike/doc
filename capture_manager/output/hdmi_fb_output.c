
#include "../common.h"
#include "output.h"
#include <linux/mxcfb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BPP 16
typedef unsigned short pix_t;

#define XRES 1920
#define YRES 1080

#define DEV_NAME "/dev/fb2"

static int g_fd;
static pix_t* g_fb = NULL;
static int g_fbsize = 0;
static output_param_t g_param;

#define RGB(r,g,b) ((r & 0x1F) << 11 | (g & 0x3F) << 5 | (b & 0x1F))

#define COLOR_WHITE (0xE79C)
#define COLOR_BLACK (0x2084)
#define COLOR_RED   (0x7DEF)

static int fb_open(output_param_t* param)
{
    int retval = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    if ((g_fd = open(DEV_NAME, O_RDWR, 0)) < 0)
    {
        log_error("Unable to open %s\n", DEV_NAME);
        return -1;
    }

    retval = ioctl(g_fd, FBIOGET_FSCREENINFO, &finfo);
    if (retval < 0)
    {
        goto errfd;
    }

    retval = ioctl(g_fd, FBIOGET_VSCREENINFO, &vinfo);
    if (retval < 0)
    {
        goto errfd;
    }

    log_info("framebuffer: screen:%dx%d, vscreen:%dx%d\n", vinfo.xres, vinfo.yres,
        vinfo.xres_virtual, vinfo.yres_virtual);
    vinfo.bits_per_pixel = BPP;
    vinfo.yoffset = 0;
    //vinfo.xoffset = 0;
    vinfo.xres = XRES;
    vinfo.yres = YRES;
    vinfo.xres_virtual = XRES;
    vinfo.yres_virtual = YRES;
    retval = ioctl(g_fd, FBIOPUT_VSCREENINFO, &vinfo);
    if (retval < 0)
    {
        goto errfd;
    }
    g_fbsize = vinfo.xres * vinfo.yres_virtual * vinfo.bits_per_pixel / 8;

    log_debug("bits_per_pixel: %d, fbsize=%d, x,yoffset=%d,%d\n", vinfo.bits_per_pixel, g_fbsize,
        vinfo.xoffset, vinfo.yoffset);

    log_debug("R: len=%d, offset=%d, msb=%d\n", vinfo.red.length, vinfo.red.offset, vinfo.red.msb_right);
    log_debug("G: len=%d, offset=%d, msb=%d\n", vinfo.green.length, vinfo.green.offset, vinfo.green.msb_right);
    log_debug("B: len=%d, offset=%d, msb=%d\n", vinfo.blue.length, vinfo.blue.offset, vinfo.blue.msb_right);

    /* Map the device to memory*/
    g_fb = (pix_t *)mmap(0, g_fbsize, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 0);
    if (g_fb == NULL)
    {
        log_error("\nError: failed to map framebuffer device 0 to memory.\n");
        goto errmap;
    }

    memcpy(&g_param, param, sizeof(g_param));
    return 0;

errmap:
    munmap(g_fb, g_fbsize);
errfd:
    close(g_fd);
    return -1;
}

static int fb_start()
{
    log_debug("%s is called\n", __func__);
    return 0;
}

static void fb_close()
{
    munmap(g_fb, g_fbsize);
    close(g_fd);
    g_fb = NULL;
    g_fbsize = 0;
    g_fd = 0;
}

#define COLOR_BG (0x33)

static int fb_write_frame(unsigned char * frame_buf, int frame_size)
{
    register int x, y, fb_offset = 0, src_offset = 0, tmp;
    //memset(g_fb, COLOR_BG, g_fbsize);
    for (y = 0; y < g_param.source_height; y++)
    {
        for (x = 0; x < g_param.source_width; x++)
        {
            tmp = frame_buf[x + src_offset];
            if (tmp < 128)
                g_fb[x + fb_offset] = COLOR_BLACK;
            else if (tmp == 0xfe)
                g_fb[x + fb_offset] = COLOR_RED;
            else
                g_fb[x + fb_offset] = COLOR_WHITE;
            //g_fb[x + fb_offset] = frame_buf[x + src_offset] >= 128 ? COLOR_WHITE : COLOR_BLACK; //0xFFFF : (COLOR_BG << 8 | COLOR_BG);
        }

        fb_offset += XRES;
        src_offset += g_param.source_width;
    }
    return 0;
}

output_device_t hdmi_fb_output_device = {
    .name = "HDMI framebuffer output",
    .open = fb_open,
    .start = fb_start,
    .close = fb_close,
    .write_frame = fb_write_frame,
    .get_next_buf = NULL
};

#ifdef __cplusplus
}
#endif

