#include "common.h"
#include "MPlayer.h"
#include "PlayerInfo.h"


void initAudioTrack(JNIEnv *env, PlayerInfo *pInfo);

void initVideoSurface(JNIEnv *env, PlayerInfo *pInfo, jobject pJobject);

extern "C"
JNIEXPORT void JNICALL
Java_com_gloomyer_player_player_VideoPalyer_play(JNIEnv *env, jclass type, jstring path_,
                                                 jobject surface) {
    const char *path = env->GetStringUTFChars(path_, 0);
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
    pthread_create(&pInfo->decode_thread, 0, decode_proc, pInfo);

    //pthread_create(&pInfo->decode_thread, nullptr, decode_audio_proc, pInfo);

    //等待线程结束
    pthread_join(pInfo->decode_thread, 0);
    //pthread_join(pInfo->audio_decode_thread, nullptr);

    release(env, pInfo);

    env->ReleaseStringUTFChars(path_, path);
}

void initVideoSurface(JNIEnv *env, PlayerInfo *pInfo, jobject surface) {
    LOGE("%s", initVideoSurface);
    pInfo->native_window = ANativeWindow_fromSurface(env, surface);
}

void initAudioTrack(JNIEnv *env, PlayerInfo *pInfo) {
    LOGE("%s", initAudioTrack);
    AVCodecContext *audio_codec_ctx = pInfo->audio_codec_ctx;

    pInfo->swr_ctx = swr_alloc();

    pInfo->in_sample_fmt = audio_codec_ctx->sample_fmt; //解码器的格式
    pInfo->out_sample_fmt = AV_SAMPLE_FMT_S16;

    pInfo->in_sample_rate_hz = audio_codec_ctx->sample_rate;
    pInfo->out_sample_rate_hz = pInfo->in_sample_rate_hz;
    LOGE("in_sample_rate_hz: %d",  pInfo->in_sample_rate_hz);
    LOGE("out_sample_rate_hz: %d", pInfo->out_sample_rate_hz);

    pInfo->in_channel_layout = audio_codec_ctx->channel_layout;
    pInfo->out_channel_layout = AV_CH_LAYOUT_STEREO;

    pInfo->out_channel_nb = av_get_channel_layout_nb_channels( pInfo->out_channel_layout);

    swr_alloc_set_opts(pInfo->swr_ctx, pInfo->out_channel_layout,
                       pInfo->out_sample_fmt,  pInfo->out_sample_rate_hz,
                       pInfo->in_channel_layout, pInfo->in_sample_fmt, pInfo->in_sample_rate_hz,
                       0, 0);

    swr_init(pInfo->swr_ctx);


    //获取audioTrack对象
    jclass player_class = env->FindClass("com/gloomyer/player/player/VideoPalyer");
    jmethodID create_audio_track_mid = env->GetStaticMethodID(player_class, "createAudioTrack",
                                                              "(II)Landroid/media/AudioTrack;");
    LOGE("find create_audio_track_mid :%d", create_audio_track_mid == 0);
    jobject audio_track_obj = env->CallStaticObjectMethod(player_class, create_audio_track_mid,
                                                          pInfo->out_sample_rate_hz,
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