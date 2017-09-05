
#include "dropframe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common.h"

static void evenly_distribute_drop_frame_flag(float fullfps, float targetfps, int flagArr[], int flagArrSize)
{
	int i, cnt = 0;
	int ifullfps = (int)(fullfps * 1000.0);
	int itargetfps = (int)(targetfps * 1000.0);
	for (i=0; i < flagArrSize; i++) {
		if (cnt * ifullfps >= (i+1) * itargetfps) {
			flagArr[i] = 0;
		}else{
			flagArr[i] = 1;
			cnt++;
		}
	}
}

int dropframe_init(DropFrameContex_t* ctx, const float full_fps, const float target_fps)
{
	memset(ctx, 0, sizeof(DropFrameContex_t));
	ctx->full_fps = full_fps;
	ctx->target_fps = target_fps;
	ctx->curr_index = 0;
	evenly_distribute_drop_frame_flag(full_fps, target_fps, ctx->flag_arr, DROP_FRAME_FLAG_ARRAY_SIZE);
}

int dropframe_get_flag(DropFrameContex_t* ctx)
{
	int flag = ctx->flag_arr[ctx->curr_index];
	if (++ctx->curr_index >= DROP_FRAME_FLAG_ARRAY_SIZE)
		ctx->curr_index = 0;
	return flag;
}

void dropframe_dump_flag(const DropFrameContex_t* ctx)
{
	int i;
	log_info("dropframe flag: ");
	for(i=0; i < DROP_FRAME_FLAG_ARRAY_SIZE; i++) {
		log_info("%d", ctx->flag_arr[i]);
	}
	log_info("\n");
}