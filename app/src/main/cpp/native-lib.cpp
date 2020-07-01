#include <jni.h>
#include <string>
#include <android/log.h>

extern "C" {
//解码
#include "libavcodec/avcodec.h"
//缩放
#include <libswscale/swscale.h>
//封装格式
#include <libavformat/avformat.h>
//重采样
#include <libswresample/swresample.h>
}

#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"LC",FORMAT,##__VA_ARGS__);
//window上有48000Hz 和41000Hz
#define MAX_AUDIO_FRME_SIZE 48000*4

extern "C" JNIEXPORT jstring JNICALL
Java_com_frizzle_frizzlemusic_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(av_version_info());
}extern "C"
JNIEXPORT void JNICALL
Java_com_frizzle_frizzlemusic_FrizzlePlayer_openAudio(JNIEnv *env, jobject thiz, jstring input_,
                                                      jstring output_) {
    const char *input = env->GetStringUTFChars(input_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);
    //初始化FFmpeg,可以不初始化
    avformat_network_init();
    //总上下文
    AVFormatContext *avFormatContext = avformat_alloc_context();
    //打开音频文件
    if(avformat_open_input(&avFormatContext,input,NULL,NULL) != 0){
        LOGE("打开文件失败");
        return;
    }
    //查找音频流
    if (avformat_find_stream_info(avFormatContext,NULL) != 0) {
        return;
    }
    int audio_stream_index;
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
            break;
        }
    }
    //获取解码器参数
    AVCodecParameters *avCodecParameters = avFormatContext->streams[audio_stream_index]->codecpar;
    //获取解码器
    AVCodec *avCodec = avcodec_find_decoder(avCodecParameters->codec_id);
    //解码器上下文
    AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);
    //将解码器参数给解码器上下文
    avcodec_parameters_to_context(avCodecContext, avCodecParameters);
    //读取数据的包
    AVPacket *avPacket = av_packet_alloc();
    //转换器上下文
    SwrContext *swrContext = swr_alloc();

    //获取输入采样位数
    AVSampleFormat in_sample_fmt = avCodecContext->sample_fmt;
    //获取输入采样率
    int in_sample_rate = avCodecContext->sample_rate;
    //获取输入通道数
    uint64_t in_channel_layout = avCodecContext->channel_layout;
    //输出参数
    //定义输出采样位数
    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //定义输出采样率
    int out_sample_rate = 44100;
    //定义输出通道
    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    //设置采样频率,采样位数,通道数
    swr_alloc_set_opts(swrContext, out_channel_layout, out_sample_fmt, out_sample_rate,
                       in_channel_layout, in_sample_fmt, in_sample_rate, 0, NULL);
    //初始化转换器
    swr_init(swrContext);
    //输出缓冲区,大小一般为输出采样率*通道数,上面用的是双通道
    uint8_t *out_buffer = (uint8_t *) (av_malloc(2 * 44100));
    //打开音频文件解压为wb格式
    FILE *fc_pcm = fopen(output, "wb");
    int count;
    //读取音频流
    while (av_read_frame(avFormatContext, avPacket) >= 0) {
        avcodec_send_packet(avCodecContext, avPacket);
        //拿到解码后的数据(未压缩数据)
        AVFrame *avFrame = av_frame_alloc();
        int result = avcodec_receive_frame(avCodecContext, avFrame);
        if (result == AVERROR(EAGAIN)) {//有错误
            LOGE("有错误");
            continue;
        } else if (result < 0) {//解码完成
            break;
        }

        if (avPacket->stream_index != audio_stream_index) {//判断是否是音频流
            continue;
        }
        LOGE("解码%d", count++);
        //将解压后的frame转换成统一格式
        swr_convert(swrContext, &out_buffer, 2 * 44100, (const uint8_t **) (avFrame->data),
                    avFrame->nb_samples);

        //out_buffer输出到文件

        //获取输出的布局通道数
        int out_nb_channel = av_get_channel_layout_nb_channels(out_channel_layout);
        //获取每一帧的实际大小
        int out_buffer_size = av_samples_get_buffer_size(NULL, out_nb_channel, avFrame->nb_samples,
                                                         out_sample_fmt, 1);
        //写入文件
        fwrite(out_buffer,1,out_buffer_size,fc_pcm);

    }

    //释放资源
    fclose(fc_pcm);
    av_free(out_buffer);
    swr_free(&swrContext);
    avcodec_close(avCodecContext);
    avformat_close_input(&avFormatContext);
    env->ReleaseStringUTFChars(input_, input);
    env->ReleaseStringUTFChars(output_, output);
}