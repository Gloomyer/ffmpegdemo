#pragma once

#include "common.h"
#include "PlayerInfo.h"

enum CODEC_TYPE {
    CODEC_VIDEO,
    CODEC_AUDIO
};

const int MAX_LENGTH = 48000 * 4;

bool init_input_format_ctx(PlayerInfo *pInfo, const char *path);

void release(JNIEnv *env, PlayerInfo *pInfo);

bool init_codec_ctx(PlayerInfo *pInfo, int stream_index, CODEC_TYPE type);

void *decode_proc(void *args);

void decode_video_proc(PlayerInfo *pInfo, AVPacket *packet, AVFrame *yuv_frame,
                       AVFrame *rgb_frame, ANativeWindow_Buffer outBuffer);

void decode_audio_proc(JNIEnv *env, PlayerInfo *pInfo, AVPacket *packet, AVFrame *audio_frame,
                       uint8_t *out_buffer);