#pragma once

#include "../common.h"

namespace GAUDIO {

#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

    //上下文
    class PlayerContext {
    public:
        unsigned char *buffer;
        size_t bufferSize;

        PlayerContext(unsigned char *buffer, size_t bufferSize) {
            this->buffer = buffer;
            this->bufferSize = bufferSize;
        }
    };

    SLObjectItf engineObj; //引擎
    SLEngineItf engineInterface; //引擎接口
    SLObjectItf outputMixObj; //混音
    SLObjectItf audioPlayerObj; //播放器
    SLAndroidSimpleBufferQueueItf androidPlayerBufferQueueItf;//缓冲器队列
    SLPlayItf audioPlayInterface; //播放器接口

    const int buffer_size = 48000 * 4;
    unsigned char *buffer;

    /**
     * 初始化引擎
     */
    void AudioPlayerInit(int channel_nb, int hz_rate, int bits);

    /**
     * 创建对象
     * @param obj
     */
    void RealizeObject(SLObjectItf obj);

    /**
     * 创建缓冲区队列
     */
    void CreateBufferQueueAudioPlayer(int channel_nb, int hz_rate, int bits);

    /**
     * 播放器回调
     * @param queue
     * @param ctx
     */
    void PlayerCallBack(SLAndroidSimpleBufferQueueItf queue, void *context);
}

