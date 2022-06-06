
#include "class_action/class_action.hpp"

using namespace std;
namespace smc {
    namespace cls_act {
        static constexpr double model_points_68[][3] = {
                {-73.393523, -29.801432, -47.667532},
                {-72.775014, -10.949766, -45.909403},
                {-70.533638, 7.929818,   -44.84258},
                {-66.850058, 26.07428,   -43.141114},
                {-59.790187, 42.56439,   -38.635298},
                {-48.368973, 56.48108,   -30.750622},
                {-34.121101, 67.246992,  -18.456453},
                {-17.875411, 75.056892,  -3.609035},
                {0.098749,   77.061286,  0.881698},
                {17.477031,  74.758448,  -5.181201},
                {32.648966,  66.929021,  -19.176563},
                {46.372358,  56.311389,  -30.77057},
                {57.34348,   42.419126,  -37.628629},
                {64.388482,  25.45588,   -40.886309},
                {68.212038,  6.990805,   -42.281449},
                {70.486405,  -11.666193, -44.142567},
                {71.375822,  -30.365191, -47.140426},
                {-61.119406, -49.361602, -14.254422},
                {-51.287588, -58.769795, -7.268147},
                {-37.8048,   -61.996155, -0.442051},
                {-24.022754, -61.033399, 6.606501},
                {-11.635713, -56.686759, 11.967398},
                {12.056636,  -57.391033, 12.051204},
                {25.106256,  -61.902186, 7.315098},
                {38.338588,  -62.777713, 1.022953},
                {51.191007,  -59.302347, -5.349435},
                {60.053851,  -50.190255, -11.615746},
                {0.65394,    -42.19379,  13.380835},
                {0.804809,   -30.993721, 21.150853},
                {0.992204,   -19.944596, 29.284036},
                {1.226783,   -8.414541,  36.94806},
                {-14.772472, 2.598255,   20.132003},
                {-7.180239,  4.751589,   23.536684},
                {0.55592,    6.5629,     25.944448},
                {8.272499,   4.661005,   23.695741},
                {15.214351,  2.643046,   20.858157},
                {-46.04729,  -37.471411, -7.037989},
                {-37.674688, -42.73051,  -3.021217},
                {-27.883856, -42.711517, -1.353629},
                {-19.648268, -36.754742, 0.111088},
                {-28.272965, -35.134493, 0.147273},
                {-38.082418, -34.919043, -1.476612},
                {19.265868,  -37.032306, 0.665746},
                {27.894191,  -43.342445, -0.24766},
                {37.437529,  -43.110822, -1.696435},
                {45.170805,  -38.086515, -4.894163},
                {38.196454,  -35.532024, -0.282961},
                {28.764989,  -35.484289, 1.172675},
                {-28.916267, 28.612716,  2.24031},
                {-17.533194, 22.172187,  15.934335},
                {-6.68459,   19.029051,  22.611355},
                {0.381001,   20.721118,  23.748437},
                {8.375443,   19.03546,   22.721995},
                {18.876618,  22.394109,  15.610679},
                {28.794412,  28.079924,  3.217393},
                {19.057574,  36.298248,  14.987997},
                {8.956375,   39.634575,  22.554245},
                {0.381549,   40.395647,  23.591626},
                {-7.428895,  39.836405,  22.406106},
                {-18.160634, 36.677899,  15.121907},
                {-24.37749,  28.677771,  4.785684},
                {-6.897633,  25.475976,  20.893742},
                {0.340663,   26.014269,  22.220479},
                {8.444722,   25.326198,  21.02552},
                {24.474473,  28.323008,  5.712776},
                {8.449166,   30.596216,  20.671489},
                {0.205322,   31.408738,  21.90367},
                {-7.198266,  30.844876,  20.328022}};
    }
}


smc::cls_act::HeadPoseInfer::HeadPoseInfer(const cv::Size &imgSize)
        : async_utils::SingletonAsync<HeadPose, shared_future<PoseAction>>() {

    int len_of_array = sizeof(model_points_68) / sizeof(model_points_68[0]);
    for (int i = 0; i < len_of_array; ++i) {
        const double *p = model_points_68[i];
        this->objectPoints.emplace_back(cv::Point3d(p[0], p[1], p[2]));
    }

    focalLength = (float) imgSize.width;
    cameraCenter[0] = (float) imgSize.width / 2.f, cameraCenter[1] = (float) imgSize.height / 2.f;
    /**初始化相机内参**/
    float cameraMatrixArr[][3]{
            {focalLength, 0.f,         cameraCenter[0]},
            {0.f,         focalLength, cameraCenter[1]},
            {0.f,         0.f,         1.f}
    };
    cameraMatrix = cv::Mat(3, 3, CV_32FC1);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            cameraMatrix.at<float>(i, j) = cameraMatrixArr[i][j];
    /**初始化畸变参数**/
    distCoeffs = cv::Mat(1, 5, CV_32FC1);
    distCoeffs = 0.f; /**0填充**/
}

void smc::cls_act::HeadPoseInfer::solvePnP(const vector<cv::Point2d> &imagePoints, cv::OutputArray rVec,
                                           cv::OutputArray tVec) {
//        for (int i = 0; i < 68; i++) {
//            cout << imagePoints[i] << " ";
//        }
//        cout << endl;
    cv::solvePnP(objectPoints, imagePoints, cameraMatrix, distCoeffs,
                 rVec,
                 tVec);
}

smc::cls_act::HeadPose smc::cls_act::HeadPoseInfer::process(const shared_future<PoseAction> &posAction) {
    auto &pa = posAction.get();
    vector<cv::Point2d> faceLandmarks;
    for (int i = 26; i < 94; i++) {
        auto &kp = pa.pose[i];
        faceLandmarks.emplace_back(kp.x, kp.y);
    }
    /**获取旋转向量和偏移向量**/
    cv::Vec3d rVec;
    cv::Vec3d tVec;
    solvePnP(faceLandmarks, rVec, tVec);
    /**获取旋转向量的欧拉角**/
    cv::Vec3d eulerAngles;
    getEulerAngles(rVec, tVec, eulerAngles);

    return {faceLandmarks,
            rVec,
            tVec,
            [=](cv::InputOutputArray frame) { drawFrameAxes(frame, rVec, tVec); },
            eulerAngles};
}

void
smc::cls_act::HeadPoseInfer::drawFrameAxes(cv::InputOutputArray frame, cv::InputArray rVec, cv::InputArray tVec) {
    cv::drawFrameAxes(frame, cameraMatrix, distCoeffs, rVec, tVec, 30);
}

void smc::cls_act::HeadPoseInfer::getEulerAngles(cv::InputArray rVec, cv::InputArray tVec,
                                                 cv::OutputArray eulerAngles) {
    cv::Mat rVecMatrix;
    cv::Rodrigues(rVec, rVecMatrix); /**旋转向量-》旋转矩阵**/
    /**make projMatrix**/
    cv::Mat projMatrix;
    cv::hconcat(rVecMatrix, tVec, projMatrix);
    /**compute euler angles**/
    cv::Mat a, b, c;
    cv::decomposeProjectionMatrix(projMatrix,
                                  a,
                                  b,
                                  c,
                                  cv::noArray(), cv::noArray(), cv::noArray(), eulerAngles);
}

