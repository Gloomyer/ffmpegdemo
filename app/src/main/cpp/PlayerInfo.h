#pragma once

#include "common.h"

class PlayerInfo {
public:
    JavaVM *jvm;

    AVFormatContext *input_codec_ctx;

    int video_stream_index = -1;
    int audio_stream_index = -1;

    AVCodecContext *video_codec_ctx;
    AVCodecContext *audio_codec_ctx;

    AVCodec *video_codec;
    AVCodec *audio_codec;

    pthread_t video_decode_thread;
    pthread_t audio_decode_thread;

    ANativeWindow *native_window;

    SwrContext *swr_ctx;
    jobject audio_track_obj;
    jmethodID audio_track_write_mid;
    int out_channel_nb;
};