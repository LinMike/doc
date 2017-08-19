
#ifndef __DROP_FRMAE_H__
#define __DROP_FRMAE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DROP_FRAME_FLAG_ARRAY_SIZE 100

typedef struct {
	float full_fps;
	float target_fps;
	int curr_index;
	int flag_arr[DROP_FRAME_FLAG_ARRAY_SIZE];
}DropFrameContex_t;

int dropframe_init(DropFrameContex_t* ctx, const float full_fps, const float target_fps);
int dropframe_get_flag(DropFrameContex_t* ctx);
void dropframe_dump_flag(const DropFrameContex_t* ctx);

#ifdef __cplusplus
}
#endif


#endif
