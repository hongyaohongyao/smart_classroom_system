//
// Created by Administrator on 2021/12/10.
//

#ifndef SMART_CLASSROOM_CPP_CLASS_ACTION_HPP
#define SMART_CLASSROOM_CPP_CLASS_ACTION_HPP

#include <vector>
#include <memory>
#include <string>
#include <future>
#include <opencv2/opencv.hpp>
#include "common/async.hpp"

namespace smc {
    //class action
    namespace cls_act {

        const int ACTION_CLASS_NUM = 19;

        typedef std::tuple<cv::Mat, cv::Rect> Input;
        struct PoseAction {
            std::vector<cv::Point3f> pose;
            float actionProbs[ACTION_CLASS_NUM];
            int rawBestPred;
            int bestPred;
            int isPassing;
        };
        typedef std::vector<std::shared_future<PoseAction>> PoseActions;

        struct AffineMatrix {
            float i2d[6];       // image to dst(network), 2x3 matrix
            float d2i[6];       // dst to image, 2x3 matrix
            void compute(const cv::Size &image_size, const cv::Rect &box, const cv::Size &net_size);

            cv::Mat i2d_mat();
        };

        class Infer {
        public:
            virtual std::shared_future<PoseAction> commit(const Input &input) = 0;

            virtual PoseActions commits(const std::vector<Input> &inputs) = 0;
        };

        std::shared_ptr<Infer> create_infer(const std::string &engine_file, int gpuid);

        struct HeadPose {
            std::vector<cv::Point2d> faceLandmarks;/**面部68关键点**/
            cv::Vec3d rVec;
            cv::Vec3d tVec;
            std::function<void(cv::InputOutputArray)> drawFrameAxes;
            cv::Vec3d eulerAngles; /**pitch俯仰角 - yaw偏航角 - roll滚转角**/
        };

        constexpr const char *CHEATING_TYPES_NAME[] = {"normal", "pass things", "low head", "look around"};

        typedef std::vector<std::shared_future<HeadPose>> HeadPoses;

        class HeadPoseInfer : public async_utils::SingletonAsync<HeadPose, std::shared_future<PoseAction>> {
            std::vector<cv::Point3d> objectPoints;
            float focalLength;
            float cameraCenter[2]{};
            cv::Mat cameraMatrix;
            cv::Mat distCoeffs;
        public:
            explicit HeadPoseInfer(const cv::Size &imgSize);

            HeadPose process(const std::shared_future<PoseAction> &args) override;

            void solvePnP(const std::vector<cv::Point2d> &imagePoints, cv::OutputArray rVec, cv::OutputArray tVec);

            void drawFrameAxes(cv::InputOutputArray frame, cv::InputArray rVec, cv::InputArray tVec);

            static void getEulerAngles(cv::InputArray rVec, cv::InputArray tVec, cv::OutputArray eulerAngles);

        };


    }; // namespace cls_act

}
#endif //SMART_CLASSROOM_CPP_CLASS_ACTION_HPP
