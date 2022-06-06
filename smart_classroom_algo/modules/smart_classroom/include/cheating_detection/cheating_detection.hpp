
#ifndef SMART_CLASSROOM_CPP_CHEATING_DETECTION_HPP
#define SMART_CLASSROOM_CPP_CHEATING_DETECTION_HPP

#include "class_action/class_action.hpp"
#include "common/async.hpp"
#include "common/models_utils.hpp"
#include "detector/yolo.hpp"
#include "video_io/video_io.hpp"
#include <future>
#include <memory>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

namespace smc {
namespace cht_det {
using namespace yolo;
using namespace models_utils;

typedef cv::Mat Input;
struct ChtState {
  int predState{};
  int cheatingState{};
  int fitTimes{};
  /**********************************************************************\
   在infer中跟踪作弊使用标记更新状态，更新的状态会与帧的奇偶保持同步，未同步的状态会被清除
  \**********************************************************************/
  bool updateState{};
};
struct Data {
  std::shared_future<bool> completed; //是否完全处理完成
  cv::Mat image;
  std::shared_future<yolo::BoxArray> bboxes;
  std::shared_future<std::vector<cv::Mat>> snapshots;
  std::shared_future<std::vector<int>> tracks;
  /***********\
  *课堂动作相关*
  \**********/
  std::shared_future<cls_act::PoseActions>
      poseActions; //身体姿态，动作分类，姿态判断
  /**头部姿态估计相关**/
  std::shared_future<cls_act::HeadPoses> headPoses;
  static constexpr int CHEATING_STATE_LEN = 3;
  std::shared_future<std::vector<ChtState>>
      cheatingStates; /**本次判断 之前的判断 确认次数**/
};

struct Info {
  int delay = 40; // 25fps=1000ms/40ms
};

class Infer {
public:
  virtual Data commit(const Input &input) = 0;
};

std::shared_ptr<Infer> createInfer();

class DataSaver {
public:
  virtual void save(const Data &data){};
};

int cheatingDetection(std::atomic_ullong &endTime,
                      video_io::VideoInput &videoInput,
                      video_io::VideoOutput &videoOutput,
                      cht_det::DataSaver &dataSaver, cht_det::Info &info);
} // namespace cht_det
} // namespace smc
#endif // SMART_CLASSROOM_CPP_CHEATING_DETECTION_HPP
