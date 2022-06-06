
#ifndef SMART_CLASSROOM_CPP_MODELS_UTILS_HPP
#define SMART_CLASSROOM_CPP_MODELS_UTILS_HPP

#include <builder/trt_builder.hpp>
#include <common/ilogger.hpp>
#include <detector/yolo.hpp>
#include <class_action/class_action.hpp>
#include <detector/scrfd.hpp>
#include <face/arcface.hpp>


#define CLIP_NUM(N, MIN_, MAX_) min((MAX_) ,max((N),(MIN_)))

#define COMPUTE_EXEC_TIME(CODE_BLOCK)\
{\
    auto __begin_timer = iLogger::timestamp_now_float();\
    {\
        CODE_BLOCK\
    }\
    auto __total_time = iLogger::timestamp_now_float() - __begin_timer;\
    printf("fps: %.2f\n", 1000.0 / __total_time);\
}
namespace smc {
namespace models_utils {

bool requires(const char *name);

std::shared_ptr<yolo::Infer> getYolo();

std::shared_ptr<cls_act::Infer> getSppe();

std::shared_ptr<scrfd::Infer> getScrfd();

std::shared_ptr<arcface::Infer> getArcface();

std::shared_ptr<yolo::Infer>
createYolo(yolo::Type type, TRT::Mode mode, const std::string &model = "yolov5s");

std::shared_ptr<cls_act::Infer> createSppe(int maxBatchSize = 1, const std::string &model = "sppe");

std::shared_ptr<scrfd::Infer>
createScrfd(const std::string &model = "scrfd_2.5g_bnkps",
            float conf_thres = 0.7);

std::shared_ptr<arcface::Infer>
createArcface(const std::string &model = "arcface_iresnet50");

const int l_pair[][2] = {
        {0,   1},
        {0,   2},
        {1,   3},
        {2,   4},  // Head
        {5,   18},
        {6,   18},
        {5,   7},
        {7,   9},
        {6,   8},
        {8,   10},  // Body
        {17,  18},
        {18,  19},
        {19,  11},
        {19,  12},
        {11,  13},
        {12,  14},
        {13,  15},
        {14,  16},
        {20,  24},
        {21,  25},
        {23,  25},
        {22,  24},
        {15,  24},
        {16,  25},  // Foot
        {26,  27},
        {27,  28},
        {28,  29},
        {29,  30},
        {30,  31},
        {31,  32},
        {32,  33},
        {33,  34},
        {34,  35},
        {35,  36},
        {36,  37},
        {37,  38},
        // Face
        {38,  39},
        {39,  40},
        {40,  41},
        {41,  42},
        {43,  44},
        {44,  45},
        {45,  46},
        {46,  47},
        {48,  49},
        {49,  50},
        {50,  51},
        {51,  52},
        // Face
        {53,  54},
        {54,  55},
        {55,  56},
        {57,  58},
        {58,  59},
        {59,  60},
        {60,  61},
        {62,  63},
        {63,  64},
        {64,  65},
        {65,  66},
        {66,  67},
        // Face
        {68,  69},
        {69,  70},
        {70,  71},
        {71,  72},
        {72,  73},
        {74,  75},
        {75,  76},
        {76,  77},
        {77,  78},
        {78,  79},
        {79,  80},
        {80,  81},
        // Face
        {81,  82},
        {82,  83},
        {83,  84},
        {84,  85},
        {85,  86},
        {86,  87},
        {87,  88},
        {88,  89},
        {89,  90},
        {90,  91},
        {91,  92},
        {92,  93},
        // Face
        {94,  95},
        {95,  96},
        {96,  97},
        {97,  98},
        {94,  99},
        {99,  100},
        {100, 101},
        {101, 102},
        {94,  103},
        {103, 104},
        {104, 105},
        // LeftHand
        {105, 106},
        {94,  107},
        {107, 108},
        {108, 109},
        {109, 110},
        {94,  111},
        {111, 112},
        {112, 113},
        {113, 114},
        // LeftHand
        {115, 116},
        {116, 117},
        {117, 118},
        {118, 119},
        {115, 120},
        {120, 121},
        {121, 122},
        {122, 123},
        {115, 124},
        {124, 125},
        // RightHand
        {125, 126},
        {126, 127},
        {115, 128},
        {128, 129},
        {129, 130},
        {130, 131},
        {115, 132},
        {132, 133},
        {133, 134},
        {134, 135}
        // RightHand
};
const cv::Scalar p_color[] = {cv::Scalar(0, 255, 255), cv::Scalar(0, 191, 255), cv::Scalar(0, 255, 102),
                              cv::Scalar(0, 77, 255), cv::Scalar(0, 255, 0),
        // Nose, LEye, REye, LEar, REar
                              cv::Scalar(77, 255, 255), cv::Scalar(77, 255, 204), cv::Scalar(77, 204, 255),
                              cv::Scalar(191, 255, 77), cv::Scalar(77, 191, 255), cv::Scalar(191, 255, 77),
        // LShoulder, RShoulder, LElbow, RElbow, LWrist, RWrist
                              cv::Scalar(204, 77, 255), cv::Scalar(77, 255, 204), cv::Scalar(191, 77, 255),
                              cv::Scalar(77, 255, 191), cv::Scalar(127, 77, 255), cv::Scalar(77, 255, 127),
        // LHip, RHip, LKnee, Rknee, LAnkle, RAnkle, Neck
                              cv::Scalar(77, 255, 255), cv::Scalar(0, 255, 255),
                              cv::Scalar(77, 204, 255),  // head, neck, shoulder
                              cv::Scalar(0, 255, 255), cv::Scalar(0, 191, 255), cv::Scalar(0, 255, 102),
                              cv::Scalar(0, 77, 255), cv::Scalar(0, 255, 0), cv::Scalar(77, 255, 255)};  // foot

const cv::Scalar line_color[] = {cv::Scalar(0, 215, 255), cv::Scalar(0, 255, 204), cv::Scalar(0, 134, 255),
                                 cv::Scalar(0, 255, 50),
                                 cv::Scalar(0, 255, 102), cv::Scalar(77, 255, 222), cv::Scalar(77, 196, 255),
                                 cv::Scalar(77, 135, 255), cv::Scalar(191, 255, 77), cv::Scalar(77, 255, 77),
                                 cv::Scalar(77, 191, 255), cv::Scalar(204, 77, 255), cv::Scalar(77, 222, 255),
                                 cv::Scalar(255, 156, 127),
                                 cv::Scalar(0, 127, 255), cv::Scalar(255, 127, 77), cv::Scalar(0, 77, 255),
                                 cv::Scalar(255, 77, 36),
                                 cv::Scalar(0, 77, 255), cv::Scalar(0, 77, 255), cv::Scalar(0, 77, 255),
                                 cv::Scalar(0, 77, 255), cv::Scalar(255, 156, 127), cv::Scalar(255, 156, 127)};

const cv::Scalar white = cv::Scalar(255, 255, 255);

void drawKeypoints136(cv::Mat image, const std::vector<cv::Point3f> &keypoints);

void drawBboxes(cv::Mat image, const yolo::BoxArray &box_array, bool draw_cls = false);


}
}

#endif //SMART_CLASSROOM_CPP_MODELS_UTILS_HPP
