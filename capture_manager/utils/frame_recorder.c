
#include "frame_recorder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common.h"

handle_t frame_recorder_new(int target_frames)
{
    frame_recorder_context_t* ctx = malloc(sizeof(frame_recorder_context_t));
    memset(ctx, 0, sizeof(frame_recorder_context_t));
    ctx->target_frames = target_frames;
    return ctx;
}

enum RecorderState frame_recorder_get_state(handle_t handle)
{
    frame_recorder_context_t* ctx = (frame_recorder_context_t*)handle;
    if (ctx->target_frames <= 0) {
        return RECORDER_STATE_UNSTART;
    } else if (ctx->frames >= ctx->target_frames) {
        return RECORDER_STATE_FULL;
    } else if (ctx->frames == 0) {
        return RECORDER_STATE_EMPTY;
    } else {
        return RECORDER_STATE_INCOMPLETE;
    }
}


int frame_recorder_append(handle_t handle, uint8_t* frame_data, int frame_size)
{
    frame_recorder_context_t* ctx = (frame_recorder_context_t*)handle;

    if (ctx->skip_frames > 0) {
        ctx->skip_frames--;
        if (ctx->skip_frames == 0) {
            log_info("skip frames finishes, start to cache frames...\n");
        }
        return 0;
    }

    enum RecorderState state = frame_recorder_get_state(ctx);
    if (state == RECORDER_STATE_FULL || state == RECORDER_STATE_UNSTART) {
        return 0;
    }

    record_item_t *item = malloc(sizeof(record_item_t));
    if (item == NULL) {
        log_error("failed to malloc record item, frame_size=%d\n", frame_size);
        return -1;
    }
    item->data = malloc(frame_size);
    if (item->data == NULL) {
        log_error("failed to malloc data for record item, frame_size=%d\n", frame_size);
        free(item);
        return -1;
    }
    memcpy(item->data, frame_data, frame_size);
    item->size = frame_size;

    if (ctx->first_frame == NULL) {
        ctx->first_frame = item;
    } else {
        ctx->last_frame->next = item;
    }
    ctx->last_frame = item;
    ctx->frames++;

    return 0;
}


int frame_recorder_flush(handle_t handle, char* filepath)
{
    int ret = 0, retry=3, nwrite, cnt;
    record_item_t *item = NULL;

    frame_recorder_context_t* ctx = (frame_recorder_context_t*)handle;
    FILE *file = fopen(filepath, "wb");
    if (file == NULL) {
        log_error("could not open file %s for frame_recorder\n", filepath);
        return -1;
    }

retry_write:
    cnt = 0;
    item = ctx->first_frame;
    while(item != NULL) {
        nwrite = fwrite(item->data, 1, item->size, file);
        if (nwrite != item->size) {
            log_error("frame_recorder: not all data been written, expected=%d, actual=%d\n", item->size, nwrite);
            if (--retry > 0) {
                fseek(file, 0, SEEK_SET);
                goto retry_write;
            } else {
                log_error("exhaust retries for flush\n");
                return -1;
            }
        }
        item = item->next;

        log_info("save frame: %d/%d\n", ++cnt, ctx->frames);
    }

    fflush(file);
    fclose(file);

    log_info("frame_reorder: %d frames have been recorded, filepath=%s\n",
        ctx->target_frames, filepath);
    return 0;
}

void frame_recorder_reset(handle_t handle)
{
    frame_recorder_context_t* ctx = (frame_recorder_context_t*)handle;
    record_item_t *next_item, *item = ctx->first_frame;
    while(item != NULL) {
        next_item = item->next;
        free(item->data);
        free(item);
        item = next_item;
    }
    memset(ctx, 0, sizeof(frame_recorder_context_t));
}

void frame_recorder_free(handle_t handle)
{
    frame_recorder_context_t* ctx = (frame_recorder_context_t*)handle;
    frame_recorder_reset(ctx);
    free(ctx);
}

void frame_recorder_set_target_frames(handle_t handle, int target_frames)
{
    frame_recorder_context_t* ctx = (frame_recorder_context_t*)handle;
    ctx->target_frames = target_frames;
}

void frame_recorder_set_skip_frames(handle_t handle, int skip_frames)
{
    frame_recorder_context_t* ctx = (frame_recorder_context_t*)handle;
    ctx->skip_frames = skip_frames;
}

