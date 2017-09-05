
#include "../common.h"
#include "output.h"


#ifdef __cplusplus
extern "C" {
#endif

static int noop_open(output_param_t* param)
{
    return 0;
}

static int noop_start()
{
    log_debug("%s is called\n", __func__);
    return 0;
}

static void noop_close()
{
}

static int noop_write_frame(unsigned char * frame_buf, int frame_size)
{
    return 0;
}

output_device_t noop_output_device = {
    .name = "noop output",
    .open = noop_open,
    .start = noop_start,
    .close = noop_close,
    .write_frame = noop_write_frame,
    .get_next_buf = NULL
};

#ifdef __cplusplus
}
#endif

