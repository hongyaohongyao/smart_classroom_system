#ifndef SMART_CLASSROOM_CPP_ARCFACE_HPP
#define SMART_CLASSROOM_CPP_ARCFACE_HPP

#include <vector>
#include <memory>
#include <string>
#include <future>
#include <opencv2/opencv.hpp>
#include <common/face_detector.hpp>

namespace smc::arcface {

    typedef cv::Mat_<float> FaceFeature;
    typedef std::tuple<cv::Mat, face_detector::Box> CommitInput;
    typedef std::vector<std::shared_future<FaceFeature>> FaceFeatures;

    class Infer {
    public:
        virtual std::shared_future<FaceFeature> commit(const CommitInput &input) = 0;

        virtual FaceFeatures commits(const std::vector<CommitInput> &inputs) = 0;
    };

    cv::Mat faceAlignment(const cv::Mat &image, const face_detector::Box &landmark);

    std::shared_ptr<Infer> createInfer(const std::string &engine_file, int gpuid = 0);

}
#endif //SMART_CLASSROOM_CPP_ARCFACE_HPP
