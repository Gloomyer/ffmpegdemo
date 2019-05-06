//
// Created by Gloomy on 2019/5/6.
//

#include "AudioPlayer.h"

void GAUDIO::AudioPlayerInit(int channel_nb, int hz_rate, int bits) {
    using namespace GAUDIO;

    //创建引擎
    SLEngineOption options[] = {{SL_ENGINEOPTION_THREADSAFE, SL_BOOLEAN_TRUE}};
    slCreateEngine(&engineObj, ARRAY_LEN(engineObj), options, 0, 0, 0);
    RealizeObject(engineObj);

    //获取引擎接口
    (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engineInterface);

    //创建混音器
    (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObj, 0, 0, 0);

    //创建缓冲
    buffer = new unsigned char[buffer_size];

    //创建缓冲区
    CreateBufferQueueAudioPlayer(channel_nb, hz_rate, bits);
    RealizeObject(audioPlayerObj);

    //获取缓冲区队列接口
    (*audioPlayerObj)->GetInterface(audioPlayerObj, SL_IID_BUFFERQUEUE, &audioPlayInterface);

    PlayerContext *ctx = new PlayerContext(buffer, buffer_size);

    (*audioPlayInterface)->RegisterCallback(audioPlayInterface,
                                            reinterpret_cast<slPlayCallback>(PlayerCallBack), ctx);

    (*audioPlayerObj)->GetInterface(audioPlayerObj, SL_IID_PLAY, &audioPlayInterface);
    (*audioPlayInterface)->SetPlayState(audioPlayInterface, SL_PLAYSTATE_PLAYING);
    PlayerCallBack(androidPlayerBufferQueueItf, ctx);

}

void GAUDIO::RealizeObject(SLObjectItf obj) {
    (*obj)->Realize(obj, SL_BOOLEAN_FALSE);
}

void GAUDIO::CreateBufferQueueAudioPlayer(int channel_nb, int hz_rate, int bits) {
    using namespace GAUDIO;
    SLDataLocator_AndroidSimpleBufferQueue dataSourceLocator = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, // 定位器类型
            1                                        // 缓冲区数
    };

    SLDataFormat_PCM dataSourceFormat = {
            SL_DATAFORMAT_PCM,        // 格式类型
            static_cast<SLuint32>(channel_nb),    // 通道数
            static_cast<SLuint32>(hz_rate * 1000), // 毫赫兹/秒的样本数
            static_cast<SLuint32>(bits),        // 每个样本的位数
            static_cast<SLuint32>(bits),        // 容器大小
            SL_SPEAKER_FRONT_CENTER,  // 通道屏蔽
            SL_BYTEORDER_LITTLEENDIAN // 字节顺序
    };

    SLDataSource dataSource = {
            &dataSourceLocator, // 数据定位器
            &dataSourceFormat   // 数据格式
    };

    SLDataLocator_OutputMix dataSinkLocator = {
            SL_DATALOCATOR_OUTPUTMIX, // 定位器类型
            outputMixObj           // 输出混合
    };

    SLDataSink dataSink = {
            &dataSinkLocator, // 定位器
            0                 // 格式
    };

    SLInterfaceID interfaceIds[] = {
            SL_IID_BUFFERQUEUE
    };

    SLboolean requiredInterfaces[] = {
            SL_BOOLEAN_TRUE // for SL_IID_BUFFERQUEUE
    };

    SLresult result = (*engineInterface)->CreateAudioPlayer(
            engineInterface,
            &audioPlayerObj,
            &dataSource,
            &dataSink,
            ARRAY_LEN(interfaceIds),
            interfaceIds,
            requiredInterfaces);


}

void GAUDIO::PlayerCallBack(SLAndroidSimpleBufferQueueItf queue, void *ctx) {
    using namespace GAUDIO;

    PlayerContext *player_ctx = (PlayerContext *) ctx;
    //读取数据
    //ssize_t readSize = wav_read_data(ctx->wav, ctx->buffer, ctx->bufferSize);
    int readSize;
    if (0 < readSize) {
        (*queue)->Enqueue(queue, buffer, readSize);
    } else {
        //destroy context
        delete[] player_ctx->buffer; //释放缓存
    }
}