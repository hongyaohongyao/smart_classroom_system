#include "cheating_detection/cheating_detection.hpp"
#include "common/async.hpp"
#include "common/curl_utils.hpp"
#include "map"
#include "tracker.h"

namespace smc {
namespace cht_det {
using namespace cls_act;
using namespace std;

class InferImpl : public Infer {
private:
  shared_ptr<yolo::Infer> yolo;

public:
  InferImpl(shared_ptr<yolo::Infer> yolo, shared_ptr<cls_act::Infer> sppe,
            const cv::Size imSize = {480, 640}, int lowHeadThreshold = -75)
      : Infer(), yolo(std::move(yolo)), sppe(std::move(sppe)), snapshots(),
        tracker(), headPoseEstimator(imSize),
        cheatingAnalysis(lowHeadThreshold) {}

private:
  /**异步开始sppe的预处理 防止等待yolo的结果以及预处理时阻塞主线程**/
  class AsyncSppeProcess
      : public async_utils::SingletonAsync<PoseActions, cv::Mat,
                                           shared_future<yolo::BoxArray>> {
  public:
    shared_ptr<cls_act::Infer> sppe;

    explicit AsyncSppeProcess(shared_ptr<cls_act::Infer> &&sppe)
        : sppe(std::move(sppe)),
          async_utils::SingletonAsync<PoseActions, cv::Mat,
                                      shared_future<yolo::BoxArray>>() {}

  protected:
    vector<shared_future<PoseAction>>
    process(const cv::Mat &image,
            const shared_future<vector<Box>> &bboxes) override {
      {
        vector<cls_act::Input> inputs;
        for (const yolo::Box &box : bboxes.get()) {
          int x = CLIP_NUM((int)box.left, 0, image.cols - 1);
          int y = CLIP_NUM((int)box.top, 0, image.rows - 1);
          int w = CLIP_NUM((int)(box.right), 0, image.cols - 1) - box.left;
          int h = CLIP_NUM((int)(box.bottom), 0, image.rows - 1) - box.top;
          auto bbox = cv::Rect(x, y, w, h);
          inputs.emplace_back(make_tuple(image, bbox));
        }
        /**处理关键点**/
        if (not inputs.empty())
          return sppe->commits(inputs);
        return {};
      }
    }

  } sppe;

  class AsyncSnapshotsProcess
      : public async_utils::SingletonAsync<vector<cv::Mat>, cv::Mat,
                                           shared_future<yolo::BoxArray>> {
  public:
    explicit AsyncSnapshotsProcess()
        : async_utils::SingletonAsync<vector<cv::Mat>, cv::Mat,
                                      shared_future<yolo::BoxArray>>() {}

  protected:
    vector<cv::Mat> process(const cv::Mat &image,
                            const shared_future<vector<Box>> &bboxes) override {
      {
        vector<cv::Mat> results;
        cv::Size fromSize(image.rows, image.cols);
        const cv::Size toSize(240, 240);
        for (const yolo::Box &box : bboxes.get()) {
          AffineMatrix affineMatrix{};
          int x = CLIP_NUM((int)box.left, 0, image.cols - 1);
          int y = CLIP_NUM((int)box.top, 0, image.rows - 1);
          int w = CLIP_NUM((int)(box.right), 0, image.cols - 1) - box.left;
          int h = CLIP_NUM((int)(box.bottom), 0, image.rows - 1) - box.top;
          affineMatrix.compute(fromSize, cv::Rect(x, y, w, h), toSize);
          cv::Mat cutMat;
          cv::Mat mMat(2, 3, CV_32F, affineMatrix.i2d); /**转换矩阵**/
          warpAffine(image, cutMat, mMat, toSize);
          results.emplace_back(cutMat);
        }
        return results;
      }
    }

  } snapshots;

  class AsyncTrackerProcess
      : public async_utils::SingletonAsync<vector<int>,
                                           shared_future<yolo::BoxArray>> {
  public:
    explicit AsyncTrackerProcess()
        : sort(), async_utils::SingletonAsync<vector<int>,
                                              shared_future<yolo::BoxArray>>() {
    }

  protected:
    Tracker sort;

    vector<int> process(const shared_future<vector<Box>> &bboxes) override {
      vector<cv::Rect> inputs;
      for (const yolo::Box &box : bboxes.get()) {
        int x = (int)box.left;
        int y = (int)box.top;
        int w = (int)(box.right - box.left);
        int h = (int)(box.bottom - box.top);
        auto bbox = cv::Rect(x, y, w, h);
        inputs.emplace_back(bbox);
      }
      return sort.Run(inputs);
    }

  } tracker;

  class AsyncHeadPoseProcess
      : public async_utils::SingletonAsync<HeadPoses,
                                           shared_future<PoseActions>> {
  public:
    explicit AsyncHeadPoseProcess(const cv::Size &imSize)
        : headPoseInfer(imSize),
          async_utils::SingletonAsync<HeadPoses, shared_future<PoseActions>>() {
    }

  protected:
    HeadPoseInfer headPoseInfer;

    HeadPoses process(const shared_future<PoseActions> &poseActions) override {
      HeadPoses results;
      for (auto &pa : poseActions.get()) {
        results.emplace_back(headPoseInfer.commit(pa));
      }
      return results;
    }
  } headPoseEstimator;

  class AsyncCheatingAnalysis
      : public async_utils::SingletonAsync<
            vector<ChtState>, shared_future<PoseActions>,
            shared_future<HeadPoses>, shared_future<vector<int>>

            > {
  public:
    explicit AsyncCheatingAnalysis(int lowHeadThreshold)
        : lowHeadThreshold(lowHeadThreshold),
          async_utils::SingletonAsync<
              vector<ChtState>, shared_future<PoseActions>,
              shared_future<HeadPoses>, shared_future<vector<int>>>() {}

  protected:
    int lowHeadThreshold;

    map<int, ChtState> trackStates;
    bool updateState{};

    vector<ChtState>
    process(const shared_future<PoseActions> &poseActions_,
            const shared_future<HeadPoses> &headPoses_,
            const shared_future<vector<int>> &tracks_) override {
      updateState = !updateState;
      const auto &poseActions = poseActions_.get();
      vector<ChtState> results(poseActions.size());
      const auto &headPoses = headPoses_.get();
      const auto &tracks = tracks_.get();

      for (int i = 0; i < results.size(); ++i) {
        const auto &pa = poseActions[i].get();
        const auto &hp = headPoses[i].get();
        int pred = pa.bestPred; /**初步的作弊判定结果**/
        if (pred == 0) {
          if (hp.eulerAngles[0] < lowHeadThreshold) {
//            INFO("low head %.3f", hp.eulerAngles[0]);
            pred = 2; /**修正是否低头的结果**/
          } else if (pa.isPassing != 0) {
            //                                pred = 1;
            //                                /**修正是否传递物品的结果**/
          }
        }
        auto &curResult = results[i];
        curResult.predState = pred;
        int trackId = tracks[i];
        auto findEle = trackStates.find(trackId);
        if (findEle != trackStates.end()) {
          auto &trackState = findEle->second;
          trackState.updateState = updateState;

          if (trackState.cheatingState == pred) {
            int fitTimes = trackState.fitTimes + 1;
            if (fitTimes > 10) {
              fitTimes = 10; // limit max fit times
            }
            trackState.fitTimes = fitTimes;

            curResult.cheatingState = pred;
            curResult.fitTimes = fitTimes;
          } else {
            int fitTimes = trackState.fitTimes - 3;
            if (fitTimes <= 0) {
              fitTimes = 0;
              trackState.cheatingState = pred;
            }
            trackState.fitTimes = fitTimes;

            curResult.cheatingState = trackState.cheatingState;
            curResult.fitTimes = fitTimes;
          }
        } else {
          curResult.cheatingState = pred;
          curResult.fitTimes = 0;
          curResult.updateState = updateState;
          trackStates[trackId] = curResult;
        }
      }
      /**remove lost state**/
      for (auto it = trackStates.begin(); it != trackStates.end();) {
        if (it->second.updateState != updateState) { // loss
          it = trackStates.erase(it);
        } else {
          it++;
        }
      }
      return results;
    }
  } cheatingAnalysis;

  Data commit(const Input &image) override {
    Data data;
    data.image = image;
    data.bboxes = yolo->commit(image);                     /**目标检测**/
    data.tracks = tracker.commit(data.bboxes);             /**目标追踪**/
    data.snapshots = snapshots.commit(image, data.bboxes); /**目标截取**/
    data.poseActions = sppe.commit(image, data.bboxes);    /**动作识别**/
    data.headPoses = headPoseEstimator.commit(data.poseActions);
    data.cheatingStates =
        cheatingAnalysis.commit(data.poseActions, data.headPoses, data.tracks);
    return data;
  }
};

shared_ptr<Infer> createInfer() {
  auto sppe = models_utils::getSppe();
  if (not sppe) {
    INFOE("SPPE Load failed");
    return nullptr;
  }
  INFO("SPPE Loaded");
  auto yolo = models_utils::getYolo();
  if (not yolo) {
    INFOE("Yolo Load failed");
    return nullptr;
  }
  INFO("Yolo Loaded");
  shared_ptr<InferImpl> instance(new InferImpl(yolo, sppe));
  return instance;
}
} // namespace cht_det
} // namespace smc