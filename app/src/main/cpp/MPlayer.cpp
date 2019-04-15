#include "MPlayer.h"

bool init_input_format_ctx(PlayerInfo *pInfo, const char *path) {
    //1.注册组件
    av_register_all();

    //2.封装格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //2.打开输入视频文件
    if (avformat_open_input(&pFormatCtx, path, nullptr, nullptr) != 0) {
        LOGE("%s", "打开输入视频文件失败");
        return false;
    }

    //赋值给player
    pInfo->input_codec_ctx = pFormatCtx;

    //3.获取视频信息
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        LOGE("%s", "获取视频信息失败");
        return false;
    }

    int video_stream_idx = -1;
    int audio_stream_idx = -1;
    int i = 0;
    for (; i < pFormatCtx->nb_streams; i++) {
        //根据类型判断，是否是视频流
        if (video_stream_idx == -1 &&
            pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
        }

        if (audio_stream_idx == -1 &&
            pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_idx = i;
        }

        if (video_stream_idx >= 0 && audio_stream_idx >= 0) {
            break;
        }
    }

    pInfo->video_stream_index = video_stream_idx;
    pInfo->audio_stream_index = audio_stream_idx;

    return true;
}


void release(JNIEnv *env, PlayerInfo *pInfo) {
    env->DeleteGlobalRef(pInfo->audio_track_obj);
    ANativeWindow_release(pInfo->native_window);
    swr_free(&pInfo->swr_ctx);
    avcodec_close(pInfo->video_codec_ctx);
    avcodec_close(pInfo->audio_codec_ctx);
    avformat_free_context(pInfo->input_codec_ctx);
}

bool init_codec_ctx(PlayerInfo *pInfo, int stream_index, CODEC_TYPE type) {
    AVFormatContext *format_ctx = pInfo->input_codec_ctx;

    //获取视频解码器
    AVCodecContext *pCodeCtx = format_ctx->streams[stream_index]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodeCtx->codec_id);
    if (pCodec == nullptr) {
        LOGE("无法找到 %d 的解码器", stream_index);
        return false;
    }

    //5.打开解码器
    if (avcodec_open2(pCodeCtx, pCodec, nullptr) < 0) {
        LOGE("无法打开 %d 的解码器", stream_index);
        return false;
    }

    if (type == CODEC_VIDEO) {
        pInfo->video_codec_ctx = pCodeCtx;
        pInfo->video_codec = pCodec;
    } else if (type == CODEC_AUDIO) {
        pInfo->audio_codec_ctx = pCodeCtx;
        pInfo->audio_codec = pCodec;
    }

    return true;
}

void *decode_proc(void *args) {
    PlayerInfo *pInfo = (PlayerInfo *) args;

    JNIEnv *env;
    pInfo->jvm->AttachCurrentThread(&env, nullptr);

    AVFormatContext *format_ctx = pInfo->input_codec_ctx;

    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    ANativeWindow_Buffer videoBuffer;
    uint8_t *audioBuffer = (uint8_t *) av_malloc(MAX_LENGTH);

    AVFrame *yuv_frame = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();
    AVFrame *audio_frame = av_frame_alloc();

    while (av_read_frame(format_ctx, packet) >= 0) {
        if (packet->stream_index == pInfo->video_stream_index) {
            decode_video_proc(pInfo, packet, yuv_frame, rgb_frame, videoBuffer);
        } else if (packet->stream_index == pInfo->audio_stream_index) {
            //decode_audio_proc(env, pInfo, packet, audio_frame, audioBuffer);
        }
    }

    av_free_packet(packet);
    av_frame_free(&audio_frame);
    av_frame_free(&yuv_frame);
    av_frame_free(&rgb_frame);
    pInfo->jvm->DetachCurrentThread();
    pthread_exit(0);
}

void decode_video_proc(PlayerInfo *pInfo, AVPacket *packet, AVFrame *yuv_frame,
                       AVFrame *rgb_frame, ANativeWindow_Buffer outBuffer) {

    AVCodecContext *codec_ctx = pInfo->video_codec_ctx;
    ANativeWindow *native_window = pInfo->native_window;
    int got_frame;
    avcodec_decode_video2(codec_ctx, yuv_frame, &got_frame, packet);
    if (got_frame) {
        ANativeWindow_setBuffersGeometry(native_window, codec_ctx->width,
                                         codec_ctx->height,
                                         WINDOW_FORMAT_RGBA_8888);
        //lock
        ANativeWindow_lock(native_window, &outBuffer, nullptr);

        //fix buffer
        //yuv420 -> rgb8888
        avpicture_fill(reinterpret_cast<AVPicture *>(rgb_frame),
                       static_cast<const uint8_t *>(outBuffer.bits), AV_PIX_FMT_RGBA,
                       codec_ctx->width,
                       codec_ctx->height);

        libyuv::I420ToARGB(yuv_frame->data[0], yuv_frame->linesize[0],
                           yuv_frame->data[2], yuv_frame->linesize[2],
                           yuv_frame->data[1], yuv_frame->linesize[1],
                           rgb_frame->data[0], rgb_frame->linesize[0],
                           codec_ctx->width, codec_ctx->height);

        //unlock
        ANativeWindow_unlockAndPost(native_window);
    }
}


void decode_audio_proc(JNIEnv *env, PlayerInfo *pInfo, AVPacket *packet, AVFrame *audio_frame,
                       uint8_t *out_buffer) {
    SwrContext *swr_ctx = pInfo->swr_ctx;
    AVCodecContext *codec_ctx = pInfo->audio_codec_ctx;
    jobject audio_track_obj = pInfo->audio_track_obj;
    jmethodID audio_track_write_mid = pInfo->audio_track_write_mid;


    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;


    int got_frame = 0;
    avcodec_decode_audio4(codec_ctx, audio_frame, &got_frame, packet);
    if (got_frame) {
        swr_convert(swr_ctx, &out_buffer, MAX_LENGTH,
                    (const uint8_t **) audio_frame->data,
                    audio_frame->nb_samples);

        int out_buffer_size = av_samples_get_buffer_size(nullptr,
                                                         pInfo->out_channel_nb,
                                                         audio_frame->nb_samples,
                                                         out_sample_fmt, 1);

        jbyteArray audio_sample_arr = env->NewByteArray(out_buffer_size);

        jbyte *p_audio_sample_arr = env->GetByteArrayElements(audio_sample_arr, nullptr);
        memcpy(p_audio_sample_arr, out_buffer, out_buffer_size);

        env->ReleaseByteArrayElements(audio_sample_arr, p_audio_sample_arr, 0);

        env->CallIntMethod(audio_track_obj, audio_track_write_mid, audio_sample_arr, 0,
                           out_buffer_size);

        env->DeleteLocalRef(audio_sample_arr);
        usleep(1000 * 16);
    }
    pthread_exit(0);
}