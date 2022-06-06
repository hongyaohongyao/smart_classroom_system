#include "video_io/video_io.hpp"


using namespace std;
#define OUTPUT_AV_ERR(ret) \
    char buf[1024] = {0};\
    av_strerror(ret, buf, sizeof(buf) - 1);\
    INFOE(buf);

smc::video_io::RtmpVideoOutput::RtmpVideoOutput(const string &url, const int width, const int height, const int fps_) {
    const char *outUrl = url.c_str();
    isOpened_ = false;
    /**输入宽高**/
    inWidth = width;
    inHeight = height;
    /**fps**/
    fps = fps_;
    if (fps <= 0) {
        fps = 15; /**opencv摄像头默认帧率**/
    }
    INFO("output fps %d", fps);
    /**转换上下文**/
    sws = sws_getCachedContext(sws,
                               inWidth, inHeight, AV_PIX_FMT_BGR24,
                               inWidth, inHeight, AV_PIX_FMT_YUV420P,
                               SWS_BICUBIC,
                               nullptr, nullptr, nullptr);
    if (!sws) {
        INFOE("sws_getCachedContext failed!");
        return;
    }
    /**输出的yuv**/
    yuv = av_frame_alloc();
    yuv->width = inWidth;
    yuv->height = inHeight;
    yuv->format = AV_PIX_FMT_YUV420P;
    yuv->pts = 0;

    int ret = av_frame_get_buffer(yuv, 32);
    if (ret < 0) {
        OUTPUT_AV_ERR(ret)
        return;
    }

    /**查找编码器**/
    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        INFOE("avcodec_find_encoder failed!");
        return;
    }
    /**创建上下文**/
    vc = avcodec_alloc_context3(codec);
    if (!vc) {
        INFOE("avcodec_alloc_context3 failed!");
        return;
    }
    /**配置编码器**/
    vc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER | AV_CODEC_FLAG_LOW_DELAY; /**全局参数，低延迟**/

    vc->codec_id = codec->id; /**编码ID**/
    vc->thread_count = 4; /**编码线程数**/

    vc->pix_fmt = AV_PIX_FMT_YUV420P; /**编码格式**/
    vc->bit_rate = 100 * 1024 * 8;  /**比特率**/
    vc->time_base = {1, fps};
    vc->framerate = {fps, 1};
    vc->width = inWidth;
    vc->height = inHeight;

    vc->gop_size = 50; /****/
    vc->max_b_frames = 0; /**不然开头几帧miss了**/

    av_opt_set(vc->priv_data, "preset", "ultrafast", 0); /**编码形式修改**/
    av_opt_set(vc->priv_data, "tune", "zerolatency", 0);/**实时编码**/

    /**打开编码器**/
    ret = avcodec_open2(vc, codec, nullptr);
    if (ret < 0) {
        cout << ret << endl;
        OUTPUT_AV_ERR(ret);
        return;
    }
    /**创建输出上下文**/
    ret = avformat_alloc_output_context2(&oct, nullptr, "flv", outUrl);
    if (!oct) {
        OUTPUT_AV_ERR(ret);
        return;
    }
    oct->flags |= AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_FLUSH_PACKETS; /**无缓存**/
    /**创建输出视频流**/
    vs = avformat_new_stream(oct, nullptr);
    if (!vs) {
        INFOE("avformat_new_stream failed");
        return;
    }
    vs->codec->codec_tag = 0;
    /**复制编码信息**/
    avcodec_parameters_from_context(vs->codecpar, vc);
    av_dump_format(oct, 0, outUrl, 1);

    /**打开IO**/
    ret = avio_open(&oct->pb, outUrl, AVIO_FLAG_WRITE);
    if (ret != 0) {
        OUTPUT_AV_ERR(ret);
        return;
    }
    /**写入头文件**/
    ret = avformat_write_header(oct, nullptr);
    if (ret < 0) {
        OUTPUT_AV_ERR(ret);
        return;
    }
    pack = av_packet_alloc();
    vPts = 0;
    /**初始化成功**/
    isOpened_ = true;
}


void smc::video_io::RtmpVideoOutput::write(cv::InputArray image) {

    const cv::Mat &frame = image.getMat();
    /**输入数据**/
    uint8_t *inData[AV_NUM_DATA_POINTERS] = {nullptr};
    inData[0] = frame.data;
    int lineSize[AV_NUM_DATA_POINTERS] = {0};
    lineSize[0] = (int) frame.elemSize() * frame.cols;
    /**转换yuv**/
    int h = sws_scale(sws, inData, lineSize, 0, inHeight,
                      yuv->data, yuv->linesize);
    if (h <= 0) {
        INFOE("sws_scale failed!");
        return;
    }
    /**编码一帧**/
    yuv->pts = vPts;
    vPts++;
    int ret = avcodec_send_frame(vc, yuv);
    if (ret != 0) {
        INFOE("encoding frame failed");
        return;
    }
    ret = avcodec_receive_packet(vc, pack);
    if (ret != 0) {
        INFOE("encoding frame to receive packed missing...");
        return;
    }
    /**推流**/
    pack->pts = av_rescale_q(pack->pts, vc->time_base, vs->time_base);
    pack->dts = av_rescale_q(pack->dts, vc->time_base, vs->time_base);
    pack->duration = av_rescale_q(pack->duration, vc->time_base, vs->time_base);
    ret = av_interleaved_write_frame(oct, pack);
    if (ret < 0) {
        INFOE("push streaming failed");
        return;
    }
}

