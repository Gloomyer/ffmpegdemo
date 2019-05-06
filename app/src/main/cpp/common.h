#pragma once

#include <jni.h>
#include <string>
#include <zconf.h>
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"

extern "C" {
#include "libavformat/avformat.h" //封装格式上下文
#include "libavcodec/avcodec.h" //解码库
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include <libavutil/imgutils.h>
#include "libyuv.h"
}

#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"Gloomy",FORMAT,##__VA_ARGS__);