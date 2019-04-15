package com.gloomyer.player.player;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.view.Surface;

public class VideoPalyer {
    static {
        System.loadLibrary("avutil-54");
        System.loadLibrary("swresample-1");
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avformat-56");
        System.loadLibrary("swscale-3");
        System.loadLibrary("postproc-53");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avdevice-56");
        System.loadLibrary("yuv");
        System.loadLibrary("player");
    }

    public native static void play(String path, Surface surface);

    /**
     * 创建声音播放器 供JNI调用
     * @param sampleRateInHz
     * @param nb_channels
     * @return
     */
    public static AudioTrack createAudioTrack(int sampleRateInHz, int nb_channels) {
        int channelConfig;
        if (nb_channels == 1) {
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        } else {
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        }
        int audioFmt = AudioFormat.ENCODING_PCM_16BIT;
        int bufferSize = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFmt);
        return new AudioTrack(
                AudioManager.STREAM_MUSIC,
                sampleRateInHz,
                channelConfig,
                audioFmt,
                bufferSize,
                AudioTrack.MODE_STREAM);
    }
}
