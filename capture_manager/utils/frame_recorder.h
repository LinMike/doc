
#ifndef __FRAME_RECORDER_H__
#define __FRAME_RECORDER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../common.h"

enum RecorderState {
    RECORDER_STATE_UNSTART = 0,
    RECORDER_STATE_EMPTY = 1,
    RECORDER_STATE_FULL = 2,
    RECORDER_STATE_INCOMPLETE = 3,
};

typedef struct record_item {
    unsigned char *data;
    int size;
    struct record_item *next;
} record_item_t;

typedef struct {
    int target_frames;
    int skip_frames;

    int frames;
    record_item_t *first_frame;
    record_item_t *last_frame;
} frame_recorder_context_t;

handle_t frame_recorder_new(int target_frames);
enum RecorderState frame_recorder_get_state(handle_t handle);
int frame_recorder_append(handle_t handle, uint8_t* frame_data, int frame_size);
int frame_recorder_flush(handle_t handle, char* filepath);
void frame_recorder_reset(handle_t handle);
void frame_recorder_free(handle_t handle);
void frame_recorder_set_target_frames(handle_t handle, int target_frames);
void frame_recorder_set_skip_frames(handle_t handle, int skip_frames);


#ifdef __cplusplus
}
#endif


#endif
