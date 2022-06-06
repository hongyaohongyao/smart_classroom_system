
#ifndef SMART_CLASSROOM_CPP_DYNAMIC_ATTENDANCE_HPP
#define SMART_CLASSROOM_CPP_DYNAMIC_ATTENDANCE_HPP

#include <common/face_detector.hpp>
#include <face/arcface.hpp>
#include <future>
#include <video_io/video_io.hpp>
#include <mutex>
#include <aws/core/external/cjson/cJSON.h>
#include <map>

namespace smc::dyn_att {

    typedef cv::Mat Input;

    struct FaceScores {
        int idx;
        float confidence;

        FaceScores() : idx(-1), confidence(0) {}

        FaceScores(int idx, float confidence) : idx(idx), confidence(confidence) {}
    };

    struct Data {
        std::shared_future<bool> completed; //是否完全处理完成,没用
        cv::Mat image;
        std::shared_future<face_detector::BoxArray> bboxes;
        std::shared_future<std::vector<cv::Mat>> snapshots;
        std::shared_future<arcface::FaceFeatures> faceFeatures;
        std::shared_future<std::vector<FaceScores>> scores;
        struct {
            std::shared_ptr<std::promise<std::vector<cv::Mat>>> snapshots_pr;
        } prom;
    };

    struct MatchInfo {
        long long int timestamp;
        float confidence;
        std::string memId;
    };

    struct MemInfo {
        std::string name;
        std::string memId;

        MemInfo(const std::string &name, const std::string &memId) : name(name), memId(memId) {}
    };

    struct Info {
        int delay = 40;
        cv::Mat featureMap;
        std::vector<MemInfo> memInfos;
        float unknownThres = 0.60;
        struct sync {
            std::mutex mutex;
            std::map<int, MatchInfo> matched;
        } sync;
    };

    class Infer {
    public:
        virtual Data commit(const Input &input) = 0;
    };

    class DataSaver {
    public:
        virtual void save(const Data &data) {};
    };

    std::shared_ptr<Infer> createInfer(smc::dyn_att::Info &info);

    std::string registerFace(const std::string &imgDir);

    cJSON *feature2json(const cv::Mat_<float> &feature);


    int dynamicAttendance(std::atomic_ullong &endTime, video_io::VideoInput &videoInput,
                          video_io::VideoOutput &videoOutput,
                          dyn_att::DataSaver &dataSaver,
                          dyn_att::Info &info);


    cv::Mat string2featureMap(const std::string &str);

    std::shared_ptr<dyn_att::Info> generateInfo(const std::string &jsonStr);
}

#endif //SMART_CLASSROOM_CPP_DYNAMIC_ATTENDANCE_HPP
