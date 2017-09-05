#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "converter.h"
#include "../contour.h"

#define ENABLE_CENTROID_CALC 1
#define ENABLE_DETAIL_PRINT  1
#define ENABLE_SAVE_FAIL_DATA 0

#define HAS_CONTOUR_ERROR(err) ((err) & 0xFF000000)
#define DEF_CONTOUR_ERROR(val) ((val) | 0xFF000000)
#define DEF_CONTOUR_STATE(val) ((val) | 0x000000FF)

#define ERROR_CONTOUR_GENERAL                   DEF_CONTOUR_ERROR(1)
#define ERROR_CONTOUR_NOT_CLOSED                DEF_CONTOUR_ERROR(2)
#define ERROR_CONTOUR_NO_FRAME_END              DEF_CONTOUR_ERROR(3)
#define ERROR_CONTOUR_NO_CONTOUR_END            DEF_CONTOUR_ERROR(4)
#define ERROR_CONTOUR_EDGE_EARLY_HORIZON        DEF_CONTOUR_ERROR(5)
#define ERROR_CONTOUR_EDGE_NO_RIGHT_AFTER_LEFT  DEF_CONTOUR_ERROR(6)
#define ERROR_CONTOUR_INVALID_ORIGIN_ADDR       DEF_CONTOUR_ERROR(7)
#define ERROR_CONTOUR_INVALID_POINT_ADDR        DEF_CONTOUR_ERROR(8)
#define ERROR_CONTOUR_NOT_ENOUGH_POINTS         DEF_CONTOUR_ERROR(9)
#define ERROR_CONTOUR_EDGE_INVALID              DEF_CONTOUR_ERROR(0xA)


#define CONTOUR_SUCCESS     (0)
#define CONTOUR_END         DEF_CONTOUR_STATE(1)

//The boundary box for a contour, after geting the boundary box of a contour, we can save
//a lot time to scan the contour block and thus quicky calculate the centroid
typedef struct {
    int minx;
    int miny;
    int maxx;
    int maxy;
} boundary_box_t;

static uint8_t* g_edge_img = NULL;

static int default_check_support(input_param_t* iparam, output_param_t* oparam)
{
    return 1; //always support
}

static uint8_t* alloc_edge_img()
{
    uint8_t* buf;
    buf = (uint8_t*)malloc(STD_IMAGE_SIZE);
    if (buf == NULL)
    {
        log_error("malloc edge image failed!\n", __func__);
        return NULL;
    }
    return buf;
}

static void reset_edge_img(uint8_t* img)
{
    memset(img, 0xff, STD_IMAGE_SIZE);
}

static int dir_to_offset(const int dir, const point_t* const last_addr, point_t* addr, const int width)
{
    addr->x = last_addr->x;
    addr->y = last_addr->y;
    addr->offset = last_addr->offset;

    switch(dir) {
    case DIR_UP:
        addr->y -= 1;
        addr->offset -= width;
        break;
    case DIR_UP_RIGHT:
        addr->y -= 1;
        addr->x += 1;
        addr->offset -= (width - 1);
        break;
    case DIR_RIGHT:
        addr->x += 1;
        addr->offset += 1;
        break;
    case DIR_DOWN_RIGHT:
        addr->x += 1;
        addr->y += 1;
        addr->offset += (width + 1);
        break;
    case DIR_DOWN:
        addr->y += 1;
        addr->offset += width;
        break;
    case DIR_DOWN_LEFT:
        addr->x -= 1;
        addr->y += 1;
        addr->offset += (width - 1);
        break;
    case DIR_LEFT:
        addr->x -= 1;
        addr->offset -= 1;
        break;
    case DIR_UP_LEFT:
        addr->x -= 1;
        addr->y -= 1;
        addr->offset -= (width + 1);
        break;
    default:
        //reach the last dir of this contour
        return -1;
    }

    return 0;
}


static int scan_edge_img_row(const uint8_t* const data, const int size, const int curr_y, int* sumpix, int* sumx, int* sumy)
{
    int i, exp, left_edge = 0, left_pos;
    int sum_x = 0, sum_y = 0, sum_pix = 0;

    for (i = 0; i < size; i++)
    {
        switch(data[i])
        {
        case EDGE_LEFT:
            left_edge = 1;
            left_pos = i;

            sum_pix++;
            sum_x += i;

            break;
        case EDGE_RIGHT:
            if (left_edge)
            {
                exp = (i - left_pos);
                sum_pix += exp;
                sum_x += (left_pos + i) * exp / 2; //等差数据列求和
                left_edge = 0;
            }
            else //this is vertex
            {
                sum_pix += 1;
                sum_x += i;
            }

            left_edge = 0;
            break;

        case EDGE_HORIZON:
            if (!left_edge)
            {
#if ENABLE_DETAIL_PRINT
                log_error("Detect horizon edge before left, impossible!\n");
#endif
                return ERROR_CONTOUR_EDGE_EARLY_HORIZON;
            }
            break;
        default:
            if (data[i] != 0xff)
            {
#if ENABLE_DETAIL_PRINT
                log_error("invalid edge type %d\n", data[i]);
#endif
                return ERROR_CONTOUR_EDGE_INVALID;
            }
            break;
        }
    }

    sum_y = sum_pix;
    *sumpix += sum_pix;
    *sumx += sum_x;
    *sumy += (sum_y * curr_y);

    return CONTOUR_SUCCESS;
}


static int calc_centroid_from_edge_image(centroid_t* centroid, const int width, const uint8_t* const edge_img, const boundary_box_t* const bbox)
{
    int ret;
    int i, num_of_rows, num_of_cols, offset;
    int sum_pix = 0, sum_x = 0, sum_y = 0;

    num_of_rows = (bbox->maxy - bbox->miny + 1);
    num_of_cols = (bbox->maxx - bbox->minx + 1);
    offset = bbox->miny * width + bbox->minx; //the start pix offset

    for (i = 0; i < num_of_rows; i++)
    {
        ret = scan_edge_img_row(edge_img + offset, num_of_cols, i, &sum_pix, &sum_x, &sum_y);
        if (HAS_CONTOUR_ERROR(ret))
        {
            log_error("Fail to scan the edge image row %d!\n", i);
            return ret;
        }
        offset += width;

        //log_debug("sum_pix=%d, sum_x=%d, sum_y=%d\n", sum_pix, sum_x, sum_y);
    }

    //log_debug(">>> sum_pix=%d, sum_x=%d, sum_y=%d\n", sum_pix, sum_x, sum_y);

    if (sum_pix == 0)
    {
        log_error("sum_pix=0\n");
        return ERROR_CONTOUR_GENERAL;
    }

    centroid->x = (float)sum_x / (float)sum_pix + bbox->minx;
    centroid->y = (float)sum_y / (float)sum_pix + bbox->miny;

    return CONTOUR_SUCCESS;
}

static void dump_edge(const boundary_box_t* const bbox, const uint8_t* const data)
{
    int i, j, cols, rows;
    int width = STD_IMAGE_WIDTH;
    uint8_t curdata;

    cols = bbox->maxx - bbox->minx + 1;
    rows = bbox->maxy - bbox->miny + 1;
    int offset = bbox->minx + bbox->miny * width;
    for (i = 0; i < rows; i++)
    {
        printf("\n");
        for (j = 0; j < cols; j++)
        {
            curdata = data[offset+j];
            //printf("%02x ", curdata);
            printf("%d", curdata != 0xff ? (curdata >> 4) : 0);
        }
        offset += width;
    }
    printf("\n");

}

static int parse_contour_edge(const uint8_t* const data, contour_t* contour, uint8_t* edge_img, boundary_box_t* bbox)
{
    int i, n;
    point_t* pt;
    const uint8_t *raw = data + CONTOUR_HEADER_SIZE;
    reset_edge_img(edge_img);

    bbox->minx = STD_IMAGE_WIDTH;
    bbox->miny = STD_IMAGE_HEIGHT;
    bbox->maxx = bbox->maxy = 0;

    //The first and last point are the same point as the contour is closed.
    //actually, if force to calculate the edge of the last point, there will be
    //something wrong, seems the FPGA doesn't record the correct edge information
    //for last point.
    //So, skip the last point
    n = contour->count - 1;
    for (i = 0; i < n; i++)
    {
        pt = &contour->points[i];
        edge_img[pt->offset] = (raw[i] & CONTOUR_EDGE_MASK);

        if (pt->x < bbox->minx)
            bbox->minx = pt->x;

        if (pt->x > bbox->maxx)
            bbox->maxx = pt->x;

        if (pt->y < bbox->miny)
            bbox->miny = pt->y;

        if (pt->y > bbox->maxy)
            bbox->maxy = pt->y;
    }

    //log_debug("bbox: (%d,%d)=>(%d,%d)\n", bbox->minx, bbox->miny, bbox->maxx, bbox->maxy);
    //dump_edge(bbox, edge_img);

    return 0;
}

static int parse_contour_points(const uint8_t* const data, contour_t* contour)
{
    int i, dir;
    point_t *addr;
    point_t origin;

    //If a line starts with 0xffffff, it means reaching the last contour
    if (data[0] == 0xff && data[1] == 0xff && data[2] == 0xff)
    {
        return CONTOUR_END; //read the last coutour
    }

    //Fetch the target address of the first point
    contour->count = 0;
    addr = &contour->points[0];

    //The first 4 bytes store the address of origin point
    addr->x = (data[0] << 8) + data[1];
    addr->y = (data[2] << 8) + data[3];
    addr->offset = addr->y * STD_IMAGE_WIDTH + addr->x;
    contour->count++;

    if (addr->x < 0 || addr->x >= STD_IMAGE_WIDTH ||
        addr->y < 0 || addr->y >= STD_IMAGE_HEIGHT)
    {
#if ENABLE_DETAIL_PRINT
        log_error("Invalid origin point, x=%d, y=%d, raw=0x%02x%02x%02x%02x\n",
            addr->x, addr->y, data[0], data[1], data[2], data[3]);
#endif
        return ERROR_CONTOUR_INVALID_ORIGIN_ADDR;
    }

    //store the origin point, so that we can use it to judge whether the contour is closed or not
    origin.x = addr->x;
    origin.y = addr->y;
    origin.offset = addr->offset;

    for (i = CONTOUR_HEADER_SIZE; i < CONTOUR_FRAME_WIDTH; i++)
    {
        dir = (data[i] & CONTOUR_DIR_MASK);
        if (dir >= DIR_END) //end of this contour
        {
            if (i <= (CONTOUR_HEADER_SIZE + 2))
            {
#if ENABLE_DETAIL_PRINT
                log_error("Not enough points at contour, numOfPoints=%d\n", i - CONTOUR_HEADER_SIZE + 1);
#endif
                return ERROR_CONTOUR_NOT_ENOUGH_POINTS;
            }

            if (origin.x != addr->x || origin.y != addr->y)
            {
#if ENABLE_DETAIL_PRINT
                log_error("Non-Closed Contour, origin(%d,%d), last(%d,%d), numOfPoints=%d\n",
                    origin.x, origin.y, addr->x, addr->y, i - CONTOUR_HEADER_SIZE + 1);
#endif
                return ERROR_CONTOUR_NOT_CLOSED;
            }
            break;
        }

        //Move to the next target address of contour point
        contour->count++;
        dir_to_offset(dir, addr, addr+1, STD_IMAGE_WIDTH);
        addr++; //current point address

        if (addr->x < 0 || addr->x >= STD_IMAGE_WIDTH ||
            addr->y < 0 || addr->y >= STD_IMAGE_HEIGHT)
        {
#if ENABLE_DETAIL_PRINT
            log_error("Invalid point address (%d, %d), offset=%d, origin=(%d,%d)\n",
                addr->x, addr->y, i, origin.x, origin.y);
            dump_byte_array(data, CONTOUR_FRAME_WIDTH);
#endif
            return ERROR_CONTOUR_INVALID_POINT_ADDR;
        }
    }

    if (i >= CONTOUR_FRAME_WIDTH)
    {
#if ENABLE_DETAIL_PRINT
        log_error("No contour ending indicator! origin(%d,%d), last(%d,%d)\n",
                    origin.x, origin.y, addr->x, addr->y);
#endif
        return ERROR_CONTOUR_NO_CONTOUR_END;
    }

    return CONTOUR_SUCCESS;
}


static int calc_centroid(const uint8_t* const data, contour_t* contour)
{
    int ret;
    boundary_box_t bbox;
    ret = parse_contour_edge(data, contour, g_edge_img, &bbox);
    if (HAS_CONTOUR_ERROR(ret))
        return ret;

    ret = calc_centroid_from_edge_image(&contour->centroid, STD_IMAGE_WIDTH, g_edge_img, &bbox);
    if (HAS_CONTOUR_ERROR(ret))
        return ret;

    contour->centroid_valid = 1;
    return CONTOUR_SUCCESS;
}

static int parse_one_camera(const uint8_t* const data, const int size, camera_contours_t* cfc, const int enable_centroid)
{
    int i, ret;
    const uint8_t* contour_data;
    contour_t* contour;

    cfc->num_of_contours = 0;
    cfc->err = 0;
    contour = &cfc->contours[0];
    //The sequence number is always stored at the last row's 4th byte (7 bit MSB),
    //so it can be extracted in advance
    cfc->seq_number = ((data[CONTOUR_FRAME_SIZE - CONTOUR_FRAME_WIDTH + 3] & 0xFE) >> 1);
    for ( i = 0; i < size; i += CONTOUR_FRAME_WIDTH)
    {
        contour_data = data + i;
        ret = parse_contour_points(contour_data, contour);
        if (ret == CONTOUR_END)
        {
            break;
        }
        else if (HAS_CONTOUR_ERROR(ret))
        {
            cfc->err = ret;
            return ret;
        }

        if (enable_centroid)
        {
            ret = calc_centroid(contour_data, contour);
            if (HAS_CONTOUR_ERROR(ret))
                return ret;
        }

        cfc->num_of_contours++;
        contour++;
    }

    if (i >= size)
    {
#if ENABLE_DETAIL_PRINT
        log_error("No frame ending detected!\n");
#endif
        return ERROR_CONTOUR_NO_FRAME_END;
    }

    return CONTOUR_SUCCESS;
}


static int calc_next_seq_num(int cur_seq_num)
{
    if (++cur_seq_num >= MAX_CONTOUR_SEQ_NUMBER)
        cur_seq_num = 0;
    return cur_seq_num;
}

static int parse_all_cameras(const uint8_t* const data, frame_contours_t* fc, int enable_centroid)
{
    int i, c, ret;
    int data_offset = 0;
#if ENABLE_DETAIL_PRINT
    static int next_seq_num = MAX_CONTOUR_SEQ_NUMBER + 1;
    int cur_seq_num;
#endif

    //initialize as error, if eventually the contour is valid, this error will be cleared.
    memset(fc, 0, sizeof(frame_contours_t));
    for ( i = 0; i < MAX_CAMERA_NUM; i++)
        fc->cameras[i].err = ERROR_CONTOUR_GENERAL;

    for ( i = 0; i < MAX_CAMERA_NUM; i++)
    {
        ret = parse_one_camera(data + data_offset, CONTOUR_FRAME_SIZE, &fc->cameras[i], enable_centroid);
        if (HAS_CONTOUR_ERROR(ret))
            return ret;
        data_offset += CONTOUR_FRAME_SIZE;

#if ENABLE_DETAIL_PRINT
        //Check the seqnumber
        cur_seq_num = fc->cameras[i].seq_number;
        if (cur_seq_num != next_seq_num && next_seq_num <= MAX_CONTOUR_SEQ_NUMBER)
            log_warn("Detect sequence number (curr=%d, expected=%d) is not continuous, there may be frames dropped by driver\n", cur_seq_num, next_seq_num);
        next_seq_num = calc_next_seq_num(cur_seq_num);
#endif
    }

    return CONTOUR_SUCCESS;
}


static void draw_square(frame_buffer_t* buf, float fx, float fy, int w, int color)
{
    int i;
    int offset = 0;
    int x = (int)(fx + 0.5);
    int y = (int)(fy + 0.5);

    if (x > w/2) { //Avoid the x is too close to the border
        offset += (x - w/2);
    }
    if (y > w/2) { //Avoid the y is too close to the border
        offset += (y - w/2) * buf->width;
    }
    //int offset = (x - w/2) + (y - w/2) * buf->width;
    for (i = 0; i < w; i++)
    {
        memset(buf->buf + offset, color, w);
        offset += buf->width;
    }
}


static int draw_camera_contours(camera_contours_t* cfc, frame_buffer_t* dst, uint8_t color)
{
    int c, i;
    contour_t* contour;
    for (c = 0; c < cfc->num_of_contours; c++)
    {
        contour = &cfc->contours[c];
        for (i = 0; i < contour->count; i++)
        {
            dst->buf[contour->points[i].offset] = color;
        }

        if (contour->centroid_valid)
        {
            draw_square(dst, contour->centroid.x, contour->centroid.y, 4, color);
        }
    }
    return 0;
}

static int draw_frame_contours(frame_contours_t* fc, frame_buffer_t* dst)
{
    memset(dst->buf, 0, dst->size);

    if (fc->cameras[0].err == CONTOUR_SUCCESS)
        draw_camera_contours(&fc->cameras[0], dst, 0xff);

    if (fc->cameras[1].err == CONTOUR_SUCCESS)
        draw_camera_contours(&fc->cameras[1], dst, 0xfe);

    return 0;
}

static frame_contours_t g_frame_contour;
static int contour_to_yuv420(frame_buffer_t* src, frame_buffer_t* dst)
{
    int ret = parse_all_cameras(src->buf, &g_frame_contour, ENABLE_CENTROID_CALC);
    if (HAS_CONTOUR_ERROR(ret))
        return ret;

    draw_frame_contours(&g_frame_contour, dst);
    return 0;
}

static int contour_to_fifo(frame_buffer_t* src, frame_buffer_t* dst)
{
    //The input dst buffer is just the frame_contours_t, so use it directly
    frame_contours_t* fc = (frame_contours_t*)dst->buf;

    int ret = parse_all_cameras(src->buf, fc, 0);
    if (HAS_CONTOUR_ERROR(ret))
        return ret;
    return 0;
}


static int contour_open(frame_buffer_t* src, frame_buffer_t* dst)
{
    g_edge_img = alloc_edge_img();
    if (g_edge_img == NULL)
        return -1;
    return 0;
}

static int contour_close()
{
    FREE(g_edge_img);
    g_edge_img = NULL;
    return 0;
}

converter_t converter_contour_to_yuv420 = {
    .name = "Contours to YUV420",
    .open = contour_open,
    .close = contour_close,
    .convert = contour_to_yuv420,
    .check_support = default_check_support
};

converter_t converter_contour_to_fifo = {
    .name = "Contour to FIFO",
    .open = contour_open,
    .close = contour_close,
    .convert = contour_to_fifo,
    .check_support = default_check_support
};

