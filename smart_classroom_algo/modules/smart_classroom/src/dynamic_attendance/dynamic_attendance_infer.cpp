#include <common/models_utils.hpp>
#include <detector/scrfd.hpp>
#include <dynamic_attendance/dynamic_attendance.hpp>
#include <face/arcface.hpp>
#include <utility>

using namespace std;
using namespace cv;
namespace smc {
namespace dyn_att {

/**
 * async process encoding
 */
struct TempPromise {
  shared_ptr<promise<vector<cv::Mat>>> snapshots;
  shared_ptr<promise<face_detector::BoxArray>> scaledBoxes;
};

class AsyncEncoding : public async_utils::SingletonAsync<arcface::FaceFeatures,
                                                         Data, TempPromise> {
  shared_ptr<arcface::Infer> encoder;

public:
  explicit AsyncEncoding(shared_ptr<arcface::Infer> encoder)
      : encoder(std::move(encoder)) {}

  vector<shared_future<Mat_<float>>> process(const Data &data,
                                             const TempPromise &prom) override {
    auto &img = data.image;

    auto bboxes = data.bboxes.get();
    vector<arcface::CommitInput> inputs;
    inputs.reserve(bboxes.size());
    vector<cv::Mat> snapshots;
    snapshots.reserve(bboxes.size());
    for (auto box : bboxes) {
      auto crop_face = scrfd::crop_face_and_landmark(img, box);
      auto snapshot =
          arcface::faceAlignment(get<0>(crop_face), get<1>(crop_face));
      snapshots.emplace_back(snapshot);
      inputs.emplace_back(crop_face);
    }
    prom.snapshots->set_value(snapshots);
    return encoder->commits(inputs);
  }
};

class AsyncPostProcessing
    : public async_utils::SingletonAsync<vector<dyn_att::FaceScores>, Data> {
  cv::Mat featuresMap;

public:
  explicit AsyncPostProcessing(cv::Mat_<float> featuresMap)
      : featuresMap(std::move(featuresMap)) {}

  vector<dyn_att::FaceScores> process(const Data &data) override {
    vector<FaceScores> results;
    auto face_features = data.faceFeatures.get();
    results.reserve(face_features.size());
    for (auto &f : face_features) {
      auto scores = Mat(featuresMap * f.get().t());
      auto *score_p = scores.ptr<float>(0);
      int label =
          (int)(std::max_element(score_p, score_p + scores.rows) - score_p);
      float match_score = max(0.0f, score_p[label]);
      results.emplace_back(label, match_score);
    }
    return results;
  }
};

class InferImpl : public Infer {
  shared_ptr<scrfd::Infer> detector;
  AsyncEncoding encoder;
  AsyncPostProcessing post_processing;

public:
  InferImpl(shared_ptr<scrfd::Infer> detector,
            shared_ptr<arcface::Infer> encoder, smc::dyn_att::Info &info)
      : detector(std::move(detector)), encoder(std::move(encoder)),
        post_processing(info.featureMap) {}

  Data commit(const Input &input) override {
    Data data;
    data.image = input;
    data.prom.snapshots_pr = make_shared<promise<vector<Mat>>>();
    data.snapshots = data.prom.snapshots_pr->get_future();
    data.bboxes = detector->commit(input);
    data.faceFeatures = encoder.commit(data, {data.prom.snapshots_pr});
    data.scores = post_processing.commit(data);
    return data;
  }
};

} // namespace dyn_att
} // namespace smc

shared_ptr<smc::dyn_att::Infer>
smc::dyn_att::createInfer(smc::dyn_att::Info &info) {
  auto detector = models_utils::getScrfd();
  if (not detector) {
    INFOE("Scrfd Load failed");
    return nullptr;
  }
  INFO("Scrfd Loaded");
  auto encoder = models_utils::getArcface();
  if (not encoder) {
    INFOE("Arcface Load failed");
    return nullptr;
  }
  INFO("Yolo Loaded");
  shared_ptr<InferImpl> instance(new InferImpl(detector, encoder, info));
  return instance;
}