package com.frizzle.frizzlemusic;

public class FrizzlePlayer {
    static {
        System.loadLibrary("frizzlemusic");
    }

    /**
     * 将音频文件解码成pcm文件,并输出到SD卡
     * @param input 音频文件的路径
     * @param output 输出文件的路径
     */
    public native void openAudio(String input, String output);
}
