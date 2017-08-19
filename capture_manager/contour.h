#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CONTOUR_H__
#define __CONTOUR_H__

#include <stdint.h>
#include "common.h"

#define CONTOUR_FRAME_WIDTH     (320)
#define CONTOUR_FRAME_HEIGHT    (64)
#define CONTOUR_FRAME_SIZE      (CONTOUR_FRAME_WIDTH * CONTOUR_FRAME_HEIGHT)
#define MAX_CONTOUR_IN_FRAME    (CONTOUR_FRAME_HEIGHT-1) //There is always a row to tell the frame information
#define CONTOUR_HEADER_SIZE     (4) //The origin address occupies the first 4 bytes
#define MAX_ONE_CONTOUR_POINTS  (CONTOUR_FRAME_WIDTH - CONTOUR_HEADER_SIZE + 1)

/**************************************
 7 6 5
 0 * 4
 1 2 3
 **************************************/
 enum ContourDir {
    DIR_LEFT = 0,
    DIR_DOWN_LEFT = 1,
    DIR_DOWN = 2,
    DIR_DOWN_RIGHT = 3,
    DIR_RIGHT = 4,
    DIR_UP_RIGHT = 5,
    DIR_UP = 6,
    DIR_UP_LEFT = 7,
    DIR_END = 8
 };

enum ContourEdgeType {
    EDGE_HORIZON = (1 << 4),
    EDGE_LEFT = (2 << 4),
    EDGE_RIGHT = (3 << 4),
    EDGE_VERTEX = EDGE_RIGHT
};

#define CONTOUR_DIR_MASK    (0x0F)
#define CONTOUR_EDGE_MASK   (0x30)

typedef struct {
    float x;
    float y;
} centroid_t;

/*
typedef struct {
    int valid_count;
    enum CameraId camera_id;
    uint32_t frame_num;
    centroid_t data[MAX_CONTOUR_IN_FRAME];
} frame_centroid_t;
*/

typedef struct {
    int count;
    point_t points[MAX_ONE_CONTOUR_POINTS];

    int centroid_valid; //only if this flag is asserted, then the centroid data is valid
    centroid_t centroid;
} contour_t;

typedef struct {
    int err; //0: no error, otherwise this frame has something wrong, all other data should be discarded
    uint8_t seq_number; //the seq_number is used to determine whether any frame is dropped by fpga or csi driver
    int num_of_contours;
    contour_t contours[MAX_CONTOUR_IN_FRAME];
} camera_contours_t;

typedef struct {
    camera_contours_t cameras[MAX_CAMERA_NUM];
} frame_contours_t;

#endif

#ifdef __cplusplus
}
#endif

