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

    pthread_t decode_thread;

    ANativeWindow *native_window;

    SwrContext *swr_ctx;
    jobject audio_track_obj;
    jmethodID audio_track_write_mid;

    AVSampleFormat in_sample_fmt;
    AVSampleFormat out_sample_fmt;

    uint64_t in_channel_layout;
    uint64_t out_channel_layout;

    int in_sample_rate_hz;
    int out_sample_rate_hz;

    int out_channel_nb;

};