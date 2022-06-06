#ifndef SMART_CLASSROOM_CPP_YOLO_HPP
#define SMART_CLASSROOM_CPP_YOLO_HPP

#include <vector>
#include <memory>
#include <string>
#include <future>
#include <opencv2/opencv.hpp>
#include <common/trt_tensor.hpp>
#include <common/object_detector.hpp>

/**
* @brief 发挥极致的性能体验
* 支持YoloX和YoloV5
*/
namespace smc {
    namespace yolo {

        using namespace object_detector;

        enum class Type : int {
            V5 = 0,
            X = 1
        };

        enum class NMSMethod : int {
            CPU = 0,         // General, for solvePnP mAP
            FastGPU = 1      // Fast NMS with a small loss of accuracy in corner cases
        };

        void image_to_tensor(const cv::Mat &image, std::shared_ptr<TRT::Tensor> &tensor, Type type, int ibatch);

        class Infer {
        public:
            virtual std::shared_future<BoxArray> commit(const cv::Mat &image) = 0;

            virtual std::vector<std::shared_future<BoxArray>> commits(const std::vector<cv::Mat> &images) = 0;
        };

        std::shared_ptr<Infer> create_infer(
                const std::string &engine_file, Type type, int gpuid,
                float confidence_threshold = 0.25f, float nms_threshold = 0.5f,
                NMSMethod nms_method = NMSMethod::FastGPU, int max_objects = 1024,
                bool use_multi_preprocess_stream = false
        );

        const char *type_name(Type type);

    } // namespace yolo
}
#endif // SMART_CLASSROOM_CPP_YOLO_HPP