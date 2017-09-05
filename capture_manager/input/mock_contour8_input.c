#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "input.h"
#include "../contour.h"

#define ENABLE_SQUARE
#define ENABLE_TRIAG
#define ENABLE_OCTAGON
#define ENABLE_LADDER
#define ENABLE_FLGAS
//#define ENABLE_FISH

static shape_context_t g_all_shape_ctx[5] = { 0 };


static long g_time_last = 0;
static uint8_t* g_buf = NULL;
static int g_bufsize = 0;
static uint32_t g_frame_num = 0;

static int g_frame_interval = 16;

#define CONTOUR_DATA_SIZE 320
#define CONTOUR_MAX_NUM   (MAX_CONTOUR_IN_FRAME + 1) //with an additonal ending line

#define FRAME_WIDTH     1280
#define FRAME_HEIGHT    720

#define FRAME_NUM_OFFSET (3)

#define get_contour_buf(n) (g_buf + CONTOUR_DATA_SIZE * (n))


static shape_context_t* new_shape_context()
{
    shape_context_t* ctx = malloc(sizeof(shape_context_t));
    memset(ctx, 0, sizeof(shape_context_t));
    return ctx;
}


static int serialize_start_point(uint8_t* buf, int x, int y)
{
    /*buf[0] = ((x & 0xff0) >> 4);
    buf[1] = ((x & 0x00f) << 4) + ((y & 0xf00) >> 8);
    buf[2] =  (y & 0x0ff);
    return 3;
    */
    buf[0] = ((x & 0xff00) >> 8);
    buf[1] = (x & 0x00ff);
    buf[2] = ((y & 0xff00) >> 8);
    buf[3] = (y & 0x00ff);
    return 4;
}

static int shape_move(shape_context_t* ctx)
{
    int distance = ctx->move_distance;

    switch(ctx->move_type)
    {
    case MOVE_UP_DOWN:
        if (ctx->dir == 0) //up
            ctx->y += 1;
        else
            ctx->y -= 1;

        if (++ctx->move_cnt >= distance)
        {
            ctx->move_cnt = 0;
            ctx->dir = (ctx->dir ? 0 : 1);
        }
        break;

    case MOVE_LEFT_RIGHT:
        if (ctx->dir == 0) //right
            ctx->x += 1;
        else
            ctx->x -= 1;

        if (++ctx->move_cnt >= distance)
        {
            ctx->move_cnt = 0;
            ctx->dir = (ctx->dir ? 0 : 1);
        }
        break;

   case MOVE_DIAG_45:
        if (ctx->dir == 0) { //down & right
            ctx->x += 1;
            ctx->y += 1;
        }
        else {
            ctx->x -= 1; //up & left
            ctx->y -= 1;
        }

        if (++ctx->move_cnt >= distance)
        {
            ctx->move_cnt = 0;
            ctx->dir = (ctx->dir ? 0 : 1);
        }
        break;

   case MOVE_DIAG_135:
        if (ctx->dir == 0) { //up & right
            ctx->x += 1;
            ctx->y -= 1;
        }
        else {
            ctx->x -= 1; //down & left
            ctx->y += 1;
        }

        if (++ctx->move_cnt >= distance)
        {
            ctx->move_cnt = 0;
            ctx->dir = (ctx->dir ? 0 : 1);
        }
        break;

    case MOVE_ZIGZAG:


        break;

    case MOVE_NONE:
        return 0;

    default:
        return -1;
    }

    serialize_start_point(ctx->buf, ctx->x, ctx->y);
    return 0;
}


/********************************

 A--------D
 |        |
 |        |
 |        |
 B--------C

 ********************************/
static int square_create(shape_context_t* ctx)
{
    int w = ctx->width;
    uint8_t* buf = ctx->buf;

    int i = 0;
    i += serialize_start_point(buf, ctx->start_x - w/2, ctx->start_y - w/2);

    memset(buf + i, (DIR_DOWN | EDGE_LEFT), w);
    i += w;

    buf[i++] = (DIR_RIGHT | EDGE_LEFT);

    memset(buf + i, (DIR_RIGHT | EDGE_HORIZON), w-1);
    i += (w-1);

    memset(buf + i, (DIR_UP | EDGE_RIGHT), w);
    i += w;

    buf[i++] = (DIR_LEFT | EDGE_RIGHT);

    memset(buf + i, (DIR_LEFT | EDGE_HORIZON), w-1);

    buf[i+w-1] = DIR_END;

    return 0;
}


/********************************
   A
  / \
 /   \
/     \
B------C

 ********************************/

static int triag_create(shape_context_t* ctx)
{
    int w = ctx->width;
    uint8_t* buf = ctx->buf;

    int i = 0;
    i += serialize_start_point(buf, ctx->start_x, ctx->start_y);

    buf[i++] = (DIR_DOWN_LEFT | EDGE_VERTEX);

    memset(buf + i, DIR_DOWN_LEFT | EDGE_LEFT, w-2);
    i += (w-2);

    buf[i++] = (DIR_RIGHT | EDGE_LEFT);

    memset(buf + i, DIR_RIGHT | EDGE_HORIZON, 2*w-2);
    i += (2*w-2);

    memset(buf + i, DIR_UP_LEFT | EDGE_RIGHT, w-1);
    i += (w-1);

    buf[i] = (DIR_LEFT | EDGE_RIGHT); //TODO: CHECK WHY NEED THIS!

    buf[i+1] = DIR_END;

    return 0;
}

/********************************
    A   H
   /----\
  /      \G
B|        |
 |        |F
C \      /
  D\____/E

 ********************************/

static int octagon_create(shape_context_t* ctx)
{
    int w = ctx->width;
    uint8_t* buf = ctx->buf;

    int i = 0;
    i += serialize_start_point(buf, ctx->start_x, ctx->start_y);

    memset(buf + i, DIR_DOWN_LEFT | EDGE_LEFT, w-1);
    i += (w-1);

    memset(buf + i, DIR_DOWN | EDGE_LEFT, w-1);
    i += (w-1);

    memset(buf + i, DIR_DOWN_RIGHT | EDGE_LEFT, w-1);
    i += (w-1);

    buf[i++] = (DIR_RIGHT | EDGE_LEFT);

    memset(buf + i, DIR_RIGHT | EDGE_HORIZON, w-2);
    i += (w-2);

    memset(buf + i, DIR_UP_RIGHT | EDGE_RIGHT, w-1);
    i += (w-1);

    memset(buf + i, DIR_UP | EDGE_RIGHT, w-1);
    i += (w-1);

    memset(buf + i, DIR_UP_LEFT | EDGE_RIGHT, w-1);
    i += (w-1);

    buf[i++] = (DIR_LEFT | EDGE_RIGHT);

    memset(buf + i, DIR_LEFT | EDGE_HORIZON, w-2);
    i += (w-2);

    buf[i] = DIR_END;
}


/*******************************************

   /B====A\
  /        \
 /          \
 C===========D

Draw sequence: A-->B-->C-->D
 *********************************************/
static int ladder_create(shape_context_t* ctx)
{
    int w = ctx->width;
    uint8_t* buf = ctx->buf;

    int i = 0;
    i += serialize_start_point(buf, ctx->start_x, ctx->start_y);

    buf[i++] = (DIR_LEFT | EDGE_RIGHT);

    memset(buf + i, DIR_LEFT | EDGE_HORIZON, w-2);
    i += (w-2);

    memset(buf + i, DIR_DOWN_LEFT | EDGE_LEFT, w-1);
    i += (w-1);

    buf[i++] = (DIR_RIGHT | EDGE_LEFT);

    memset(buf + i, DIR_RIGHT | EDGE_HORIZON, 3*w-4);
    i += (3*w-4);

    memset(buf + i, DIR_UP_LEFT | EDGE_RIGHT, w-1);
    i += (w-1);

    buf[i] = DIR_END;

    return 0;
}

/*******************************************

A-------F
|       |
|       |
B___C   |
     |  |
     |  |
     |  |
     D__E
Draw sequence: A-->B-->C-->D
 *********************************************/
static int flags_create(shape_context_t* ctx)
{
    int w = ctx->width;
    uint8_t* buf = ctx->buf;

    int i = 0;
    i += serialize_start_point(buf, ctx->start_x, ctx->start_y);

    memset(buf + i, DIR_DOWN | EDGE_LEFT, w-1);
    i += (w-1);

    buf[i++] = (DIR_RIGHT | EDGE_LEFT);

    memset(buf + i, DIR_RIGHT | EDGE_HORIZON, w-1);
    i += (w-1);

    buf[i++] = (DIR_DOWN | EDGE_HORIZON);

    memset(buf + i, DIR_DOWN | EDGE_LEFT, w-2);
    i += (w-2);

    buf[i++] = (DIR_RIGHT | EDGE_LEFT);

    memset(buf + i, DIR_RIGHT | EDGE_HORIZON, w-2);
    i += (w-2);

    memset(buf + i, DIR_UP | EDGE_RIGHT, 2*w-2);
    i += (2*w-2);

    buf[i++] = (DIR_LEFT | EDGE_RIGHT);

    memset(buf + i, DIR_LEFT | EDGE_HORIZON, 2*w-2);
    i += (2*w-2);

    buf[i] = DIR_END;

    return 0;
}

/*******************************************

 *********************************************/
static int fish_create(shape_context_t* ctx)
{
    int w = ctx->width;
    uint8_t* buf = ctx->buf;

    int i = 0;
    i += serialize_start_point(buf, ctx->start_x, ctx->start_y);

    memset(buf + i, DIR_DOWN | EDGE_LEFT, w);
    i += w;

    memset(buf + i, DIR_DOWN_RIGHT | EDGE_LEFT, w-1);
    buf[i+w-1] = (DIR_DOWN_RIGHT | EDGE_VERTEX);
    i += w;

    memset(buf + i, DIR_UP_RIGHT | EDGE_RIGHT, w-1);
    buf[i+w-1] = (DIR_UP_RIGHT, EDGE_HORIZON);
    i += w;

    memset(buf + i, DIR_DOWN_RIGHT | EDGE_LEFT, w-1);
    buf[i+w-1] = (DIR_DOWN_RIGHT | EDGE_VERTEX);
    i += w;

    memset(buf + i, DIR_UP | EDGE_RIGHT, 3*w-1);
    buf[i+3*w-1] = (DIR_UP | EDGE_VERTEX);
    i += 3*w;

    memset(buf + i, DIR_DOWN_LEFT | EDGE_LEFT, w-1);
    buf[i+w-1] = (DIR_DOWN_LEFT, EDGE_HORIZON);
    i += w;

    memset(buf + i, DIR_UP_LEFT | EDGE_RIGHT, w-1);
    buf[i+w-1] = (DIR_UP_LEFT | EDGE_VERTEX);
    i += w;

    memset(buf + i, DIR_DOWN_LEFT | EDGE_LEFT, w);
    i += w;

    buf[i] = DIR_END;

    return 0;
}


static void end_and_fill_frame_info(uint8_t* buf)
{
    buf[0] = 0xff;
    buf[1] = 0xff;
    buf[2] = 0xff;
    buf[3] = 0; //3-6: frame number
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 1; //camera id
}

static int mock_open(input_param_t* param)
{
    int i, c = 0;

    g_frame_interval = 1000 / param->framerate;

    g_frame_num = 0;
    g_bufsize = CONTOUR_DATA_SIZE*CONTOUR_MAX_NUM;
    g_buf = (uint8_t*)malloc(g_bufsize);
    memset(g_buf, 0, g_bufsize);

    c = 0;
    CLEAR(g_all_shape_ctx);

#ifdef ENABLE_SQUARE
    shape_context_t* square = &g_all_shape_ctx[c];
    square->start_x = 200;
    square->start_y = 100;
    square->width = 60;
    square->move_type = MOVE_DIAG_45;
    square->move_distance = 180;
    square->buf = get_contour_buf(c);
    square_create(square);

    c++;
#endif

#ifdef ENABLE_TRIAG
    shape_context_t* triag = &g_all_shape_ctx[c];
    triag->start_x = 400;
    triag->start_y = 200;
    triag->width = 50;
    triag->move_type = MOVE_DIAG_135;
    triag->move_distance = 200;
    triag->buf = get_contour_buf(c);
    triag_create(triag);

    c++;
#endif

#ifdef ENABLE_OCTAGON
    shape_context_t* octagon = &g_all_shape_ctx[c];
    octagon->start_x = 400;//FRAME_WIDTH*3/4;
    octagon->start_y = 300;//FRAME_HEIGHT*3/4;
    octagon->width = 40;
    octagon->move_type = MOVE_LEFT_RIGHT;
    octagon->move_distance = 600;
    octagon->buf = get_contour_buf(c);
    octagon_create(octagon);

    c++;
#endif

#ifdef ENABLE_LADDER
    shape_context_t* ladder = &g_all_shape_ctx[c];
    ladder->start_x = 800;//FRAME_WIDTH*3/4;
    ladder->start_y = 250;//FRAME_HEIGHT*3/4;
    ladder->width = 50;
    ladder->move_type = MOVE_UP_DOWN;
    ladder->move_distance = 400;
    ladder->buf = get_contour_buf(c);
    ladder_create(ladder);

    c++;
#endif

#ifdef ENABLE_FLGAS
    shape_context_t* flags = &g_all_shape_ctx[c];
    flags->start_x = 300;//FRAME_WIDTH*3/4;
    flags->start_y = 400;//FRAME_HEIGHT*3/4;
    flags->width = 40;
    flags->move_type = MOVE_DIAG_45;
    flags->move_distance = 200;
    flags->buf = get_contour_buf(c);
    flags_create(flags);

    c++;
#endif

#ifdef ENABLE_FISH
    shape_context_t* fish = &g_all_shape_ctx[c];
    fish->start_x = 800;//FRAME_WIDTH*3/4;
    fish->start_y = 300;//FRAME_HEIGHT*3/4;
    fish->width = 20;
    fish->move_type = MOVE_LEFT_RIGHT;
    fish->move_distance = 200;
    fish->buf = get_contour_buf(c);
    fish_create(fish);

    c++;
#endif


    for (i = 0; i < sizeof(g_all_shape_ctx) / sizeof(shape_context_t); i++)
    {
        g_all_shape_ctx[i].x = g_all_shape_ctx[i].start_x;
        g_all_shape_ctx[i].y = g_all_shape_ctx[i].start_y;
    }

    //draw_equal_triag(get_contour_buf(c++), FRAME_WIDTH/2, FRAME_HEIGHT/2, 80);
    //draw_octagon(get_contour_buf(c++), FRAME_WIDTH*3/4, FRAME_HEIGHT*3/4, 40);

    end_and_fill_frame_info(get_contour_buf(c));

    return 0;
}

static int mock_start()
{
    return 0;
}

static void mock_close()
{
    FREE(g_buf);
    g_bufsize = 0;
    g_frame_num = 0;
}

static void update_frame_number(uint8_t* buf, const uint32_t num)
{
    int i;
    uint8_t* last_line = NULL;
    for (i = 0; i < g_bufsize; i+=CONTOUR_DATA_SIZE)
    {
        if (buf[i] == 0xff && buf[i+1] == 0xff && buf[i+2] == 0xff)
        {
            last_line = (buf + i);
            break;
        }
    }

    if (last_line == NULL)
    {
        log_error("%s: failed to detect the last line!\n", __func__);
        return;
    }

    memcpy(last_line + FRAME_NUM_OFFSET, (uint8_t*)&num, sizeof(uint32_t));
}

static int mock_read_frame(uint8_t** framebuf, int* framesize)
{
    int i;
    long tcur = get_time_ms();
    if (g_time_last == 0 || (tcur - g_time_last) >= g_frame_interval) {
        ++g_frame_num;
        update_frame_number(g_buf, g_frame_num);

        for (i = 0; i < sizeof(g_all_shape_ctx) / sizeof(shape_context_t); i++)
        {
            if (g_all_shape_ctx[i].buf != NULL && g_all_shape_ctx[i].width != 0) //judge valid shape
            {
                shape_move(&g_all_shape_ctx[i]);
                //dump_byte_array(g_all_shape_ctx[i].buf, CONTOUR_DATA_SIZE);
            }
        }

        *framebuf = g_buf;
        *framesize = g_bufsize;
        g_time_last = tcur;
        return g_bufsize;
    }
    return 0;
}

input_device_t mock_contour8_input_device = {
    .name = "mock contour (8bits) input",
    .start = mock_start,
    .open = mock_open,
    .close = mock_close,
    .read_frame = mock_read_frame
};


