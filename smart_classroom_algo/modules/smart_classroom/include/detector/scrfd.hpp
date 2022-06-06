
#ifndef SMART_CLASSROOM_CPP_SCRFD_HPP
#define SMART_CLASSROOM_CPP_SCRFD_HPP

#include <future>
#include "common/face_detector.hpp"
#include <vector>

namespace smc {
    namespace scrfd {
        using namespace smc::face_detector;

        class Infer {
        public:
            virtual std::shared_future<BoxArray> commit(const cv::Mat &image) = 0;

            virtual std::vector<std::shared_future<BoxArray>> commits(
                    const std::vector<cv::Mat> &images
            ) = 0;
        };

        std::tuple<cv::Mat, Box> crop_face_and_landmark(
                const cv::Mat &image, const Box &box, float scale_box = 1.5f
        );

        std::shared_ptr<Infer>
        createInfer(const std::string &engine_file, int gpuid, float confidence_threshold = 0.5f,
                    float nms_threshold = 0.5f);
    }
}
#endif //SMART_CLASSROOM_CPP_SCRFD_HPP
