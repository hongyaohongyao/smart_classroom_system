#ifndef SMART_CLASSROOM_CPP_VIDEO_IO_HPP
#define SMART_CLASSROOM_CPP_VIDEO_IO_HPP

#include <opencv2/opencv.hpp>
#include "common/ilogger.hpp"
#include "common/async.hpp"

extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil\opt.h>
#include <utility>
}

#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avformat.lib")
namespace smc::video_io {
    enum VIDEO_TYPE {
        OPENCV_CAPTURE = 1
    };

    class VideoInput {
    public:
        virtual bool read(cv::OutputArray image) = 0;

        virtual bool isOpened() { return true; }
    };

    class OpencvVideoCaptureWrapper : public VideoInput {
        bool is_loop{};
    public:
        cv::VideoCapture capture;

        explicit OpencvVideoCaptureWrapper(cv::VideoCapture &&capture, bool is_loop = false) : capture(capture),
                                                                                               is_loop(is_loop) {
            INFO("OpencvVideoCaptureWrapper Created");
        }

        ~OpencvVideoCaptureWrapper() {
            INFO("close opencv capture wrapper");
            capture.release();
        }

        bool read(cv::OutputArray image) override {
            bool ret = capture.read(image);
            if (is_loop && not ret) {
                capture.set(cv::CAP_PROP_POS_FRAMES, 0);
                ret = capture.read(image);
            }
            return ret;
        }

        bool isOpened() override {
            return capture.isOpened();
        }
    };

    /**
     * video output
     */

    class VideoOutput {
    public:
        virtual void write(cv::InputArray image) = 0;

        virtual bool isOpened() { return true; }
    };

    class ShowVideoOutput : public VideoOutput {
        std::string winName;
    public:
        explicit ShowVideoOutput(std::string winName) : winName(std::move(winName)) {

        }

        void write(cv::InputArray image) override {
            cv::imshow(winName, image);
        };
    };

    class OpencvVideoWriterWrapper : public VideoOutput {
        cv::VideoWriter writer;
    public:
        explicit OpencvVideoWriterWrapper(cv::VideoWriter &&writer) : writer(writer) {}

        ~OpencvVideoWriterWrapper() {
            writer.release();
        }

        void write(cv::InputArray image) override {
            writer.write(image);
        }

        bool isOpened() override {
            return writer.isOpened();
        }
    };

    class RtmpVideoOutput : public VideoOutput {
    private:
        std::string url;
        SwsContext *sws{}; /**创建转换上下文**/
        AVFrame *yuv{}; /**frame**/
        AVCodecContext *vc{}; /**编码器上下文**/
        AVFormatContext *oct{}; /**输出上下文**/
        AVDictionary *opt{}; /**推流参数**/
        AVStream *vs{};/**输出的视频流**/
        int inWidth, inHeight;
        int fps;
        bool isOpened_;
        AVPacket *pack{};
        int vPts;/**全局帧号**/

    public:
        explicit RtmpVideoOutput(const std::string &url, int width, int height, int fps_);

        ~RtmpVideoOutput() {
            INFO("close rtmp video output");
        }

        bool isOpened() override {
            return isOpened_;
        }

        void write(cv::InputArray image) override;
    };
}
#endif //SMART_CLASSROOM_CPP_VIDEO_IO_HPP
