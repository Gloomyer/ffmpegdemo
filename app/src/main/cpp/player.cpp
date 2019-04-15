#include "common.h"
#include "MPlayer.h"
#include "PlayerInfo.h"


void initAudioTrack(JNIEnv *env, PlayerInfo *pInfo);

void initVideoSurface(JNIEnv *env, PlayerInfo *pInfo, jobject pJobject);

extern "C"
JNIEXPORT void JNICALL
Java_com_gloomyer_player_player_VideoPalyer_play(JNIEnv *env, jclass type, jstring path_,
                                                 jobject surface) {
    const char *path = env->GetStringUTFChars(path_, nullptr);
    auto *pInfo = new PlayerInfo;
    env->GetJavaVM(&pInfo->jvm);

    //初始化视频上下文
    init_input_format_ctx(pInfo, path);
    //打开视频解码器
    init_codec_ctx(pInfo, pInfo->video_stream_index, CODEC_VIDEO);
    //打开音频解码器
    init_codec_ctx(pInfo, pInfo->audio_stream_index, CODEC_AUDIO);


    initVideoSurface(env, pInfo, surface);
    initAudioTrack(env, pInfo);
    pthread_create(&pInfo->video_decode_thread, nullptr, decode_proc, pInfo);

    //pthread_create(&pInfo->video_decode_thread, nullptr, decode_audio_proc, pInfo);

    //等待线程结束
    pthread_join(pInfo->video_decode_thread, nullptr);
    //pthread_join(pInfo->audio_decode_thread, nullptr);

    release(env, pInfo);

    env->ReleaseStringUTFChars(path_, path);
}

void initVideoSurface(JNIEnv *env, PlayerInfo *pInfo, jobject surface) {
    pInfo->native_window = ANativeWindow_fromSurface(env, surface);
}

void initAudioTrack(JNIEnv *env, PlayerInfo *pInfo) {
    AVCodecContext *audio_codec_ctx = pInfo->audio_codec_ctx;

    pInfo->swr_ctx = swr_alloc();

    AVSampleFormat in_sample_fmt = audio_codec_ctx->sample_fmt; //解码器的格式
    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;

    int in_sample_rate_hz = audio_codec_ctx->sample_rate;
    int out_sample_rate_hz = in_sample_rate_hz;

    uint64_t in_ch_layout = audio_codec_ctx->channel_layout;

    swr_alloc_set_opts(pInfo->swr_ctx,
                       AV_CH_LAYOUT_STEREO,
                       out_sample_fmt,
                       out_sample_rate_hz,
                       in_ch_layout,
                       in_sample_fmt,
                       in_sample_rate_hz,
                       0, nullptr);

    swr_init(pInfo->swr_ctx);
    pInfo->out_channel_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);


    //获取audioTrack对象
    jclass player_class = env->FindClass("com/gloomyer/player/player/VideoPalyer");
    jmethodID create_audio_track_mid = env->GetStaticMethodID(player_class, "createAudioTrack",
                                                              "(II)Landroid/media/AudioTrack;");
    LOGE("find create_audio_track_mid :%d", create_audio_track_mid == nullptr);
    jobject audio_track_obj = env->CallStaticObjectMethod(player_class, create_audio_track_mid,
                                                          out_sample_rate_hz,
                                                          pInfo->out_channel_nb);
    pInfo->audio_track_obj = env->NewGlobalRef(audio_track_obj);
    jclass audio_track_class = env->GetObjectClass(audio_track_obj);
    jmethodID audio_track_play_mid = env->GetMethodID(audio_track_class, "play",
                                                      "()V");
    env->CallVoidMethod(audio_track_obj, audio_track_play_mid);
    LOGE("%s", "audioTrack play 成功!");
    //获取write mid
    jmethodID audio_track_write_mid = env->GetMethodID(audio_track_class, "write",
                                                       "([BII)I");
    pInfo->audio_track_write_mid = audio_track_write_mid;
}