
#include "common/models_utils.hpp"

using namespace std;
using namespace smc;

bool smc::models_utils::requires(const char *name) {

  auto onnx_file = iLogger::format("%s.onnx", name);
  if (not iLogger::exists(onnx_file)) {
    INFO("Auto download %s", onnx_file.c_str());
    system(iLogger::format("wget http://zifuture.com:1556/fs/25.shared/%s",
                           onnx_file.c_str())
               .c_str());
  }

  bool exists = iLogger::exists(onnx_file);
  if (not exists) {
    INFOE("Download %s failed", onnx_file.c_str());
  }
  return exists;
}

std::shared_ptr<yolo::Infer> smc::models_utils::getYolo() {
  static auto yolo =
      models_utils::createYolo(yolo::Type::V5, TRT::Mode::FP32, "yolov5s");
  return yolo;
}

std::shared_ptr<cls_act::Infer> smc::models_utils::getSppe() {
  static auto sppe = models_utils::createSppe(60, "sppe_cht");
  return sppe;
}

std::shared_ptr<scrfd::Infer> smc::models_utils::getScrfd() {
  static auto scrfd = models_utils::createScrfd("scrfd_2.5g_bnkps", 0.7);
  return scrfd;
}

std::shared_ptr<arcface::Infer> smc::models_utils::getArcface() {
  static auto arcface = models_utils::createArcface("arcface_iresnet50");
  return arcface;
}

shared_ptr<yolo::Infer> smc::models_utils::createYolo(yolo::Type type,
                                                      TRT::Mode mode,
                                                      const string &model) {
  int deviceid = 0;
  auto mode_name = TRT::mode_string(mode);
  TRT::set_device(deviceid);

  auto int8process = [=](int current, int count, const vector<string> &files,
                         shared_ptr<TRT::Tensor> &tensor) {
    INFO("Int8 %d / %d", current, count);

    for (int i = 0; i < files.size(); ++i) {
      auto image = cv::imread(files[i]);
      yolo::image_to_tensor(image, tensor, type, i);
    }
  };

  const char *name = model.c_str();
  INFO("===================== test %s %s %s ==================================",
       yolo::type_name(type), mode_name, name);

  if (not requires(name))
    return nullptr;

  string onnx_file = iLogger::format("%s.onnx", name);
  string model_file = iLogger::format("%s.%s.trtmodel", name, mode_name);
  int test_batch_size = 1;

  if (not iLogger::exists(model_file)) {
    INFO("compiling yolo");
    TRT::compile(mode,            // FP32、FP16、INT8
                 test_batch_size, // max batch size
                 onnx_file,       // source
                 model_file,      // save to
                 {}, int8process, "inference");
  }

  INFO("Loading Yolo");
  return yolo::create_infer(
      model_file,               // engine file
      type,                     // yolo type, Yolo::Type::V5 / Yolo::Type::X
      deviceid,                 // gpu id
      0.25f,                    // confidence threshold
      0.45f,                    // nms threshold
      yolo::NMSMethod::FastGPU, // NMS method, fast GPU / CPU
      1024,                     // max objects
      false                     // preprocess use multi stream
  );
}

std::shared_ptr<cls_act::Infer>
smc::models_utils::createSppe(int maxBatchSize, const string &model) {
  TRT::set_device(0);
  INFO("===================== test alphapose fp32 "
       "==================================");

  const char *name = model.c_str();
  if (not requires(name))
    return nullptr;

  string onnx_file = iLogger::format("%s.onnx", name);
  string model_file = iLogger::format("%s.FP32.trtmodel", name);

  if (!iLogger::exists(model_file)) {
    INFO("compiling alphapose");
    TRT::compile(TRT::Mode::FP32, // FP32、FP16、INT8
                 maxBatchSize,    // max_batch_size
                 onnx_file,       // source
                 model_file       // save to
    );
  }
  INFO("Loading SPPE");
  return cls_act::create_infer(model_file, 0);
}

namespace smc {
namespace models_utils {
static string compileScrfd(const string &model, int input_width,
                           int input_height, TRT::Mode mode = TRT::Mode::FP32);
}
} // namespace smc

static string smc::models_utils::compileScrfd(const string &model,
                                              int input_width, int input_height,
                                              TRT::Mode mode) {
  const char *name = model.c_str();
  if (not requires(name))
    return "";

  string onnx_file = iLogger::format("%s.onnx", name);
  string model_file = iLogger::format("%s.%dx%d.%s.trtmodel", name, input_width,
                                      input_height, TRT::mode_string(mode));
  int test_batch_size = 6;

  if (iLogger::exists(model_file))
    return model_file;

  input_width = iLogger::upbound(input_width);
  input_height = iLogger::upbound(input_height);
  TRT::set_layer_hook_reshape(
      [&](const string &name, const std::vector<int64_t> &shape) {
        INFOV("%s, %s", name.c_str(), iLogger::join_dims(shape).c_str());
        vector<string> layerset{"Reshape_108", "Reshape_110", "Reshape_112",
                                "Reshape_126", "Reshape_128", "Reshape_130",
                                "Reshape_144", "Reshape_146", "Reshape_148"};
        vector<int> strides{8, 8, 8, 16, 16, 16, 32, 32, 32};
        auto layer_iter =
            std::find_if(layerset.begin(), layerset.end(),
                         [&](const string &item) { return item == name; });
        if (layer_iter != layerset.end()) {
          int pos = layer_iter - layerset.begin();
          int stride = strides[pos];
          return vector<int64_t>{
              -1, input_height * input_width / stride / stride * 2, shape[2]};
        }
        return shape;
      });
  INFO("Compiling scrfd");
  if (TRT::compile(TRT::Mode::FP32, // FP32、FP16、INT8
                   test_batch_size, // max batch size
                   onnx_file,       // source
                   model_file,      // save to
                   {TRT::InputDims({1, 3, input_height, input_width})})) {
    return model_file;
  } else {
    return "";
  }
}

std::shared_ptr<scrfd::Infer>
smc::models_utils::createScrfd(const std::string &model, float conf_thres) {
  TRT::set_device(0);
  string model_file = compileScrfd(model, 640, 640);
  if (model_file.empty())
    return nullptr;
  INFO("Loading scrfd");
  return scrfd::createInfer(model_file, 0, conf_thres);
}

std::shared_ptr<arcface::Infer>
smc::models_utils::createArcface(const std::string &model) {
  TRT::set_device(0);
  if (not requires(model.c_str()))
    return nullptr;

  string onnx_file = iLogger::format("%s.onnx", model.c_str());
  string model_file = iLogger::format("%s.FP32.trtmodel", model.c_str());
  int test_batch_size = 50;

  if (not iLogger::exists(model_file)) {
    INFO("compiling arcface");
    if (TRT::compile(TRT::Mode::FP32, // FP32、FP16、INT8
                     test_batch_size, // max batch size
                     onnx_file,       // source
                     model_file       // saveto
                     )) {
      return nullptr;
    }
  }
  INFO("Loading arcface");
  return arcface::createInfer(model_file, 0);
}

void smc::models_utils::drawKeypoints136(cv::Mat image,
                                         const vector<cv::Point3f> &keypoints) {
  // cout << keypoints.size() << endl;
  int width = image.cols, height = image.rows;
  //    const int kps_num = 94;
  const int kps_num[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 17, 18, 19};
  float vis_thres = 0.0001;
  int part_line[136][2];
  memset(part_line, -1, sizeof(part_line));
  int len_p_color = sizeof(p_color) / sizeof(p_color[0]);
  int len_line_color = sizeof(line_color) / sizeof(line_color[0]);
  for (int i : kps_num) {
    cv::Point3f kp = keypoints[i];
    if (kp.z < vis_thres) {
      continue;
    }
    int x = (int)kp.x, y = (int)kp.y;
    part_line[i][0] = x;
    part_line[i][1] = y;
    if (i < len_p_color) {
      cv::circle(image, cv::Point(x, y), 3, p_color[i], -1);
    } else {
      cv::circle(image, cv::Point(x, y), 1, white, 2);
    }
  }
  // Draw limbs
  int len_l_pair = sizeof(l_pair) / sizeof(l_pair[0]);
  for (int i = 0; i < len_l_pair; ++i) {
    int start_p = l_pair[i][0], end_p = l_pair[i][1];
    if (part_line[start_p][0] >= 0 and part_line[end_p][0] >= 0) {
      cv::Point start_xy =
          cv::Point(part_line[start_p][0], part_line[start_p][1]);
      cv::Point end_xy = cv::Point(part_line[end_p][0], part_line[end_p][1]);
      if (i < len_line_color) {
        int line_thickness = (int)(keypoints[start_p].z + keypoints[end_p].z);
        cv::line(image, start_xy, end_xy, line_color[i],
                 1 + 2 * line_thickness);
      } else {
        cv::line(image, start_xy, end_xy, white, 1);
      }
    }
  }
}

void smc::models_utils::drawBboxes(cv::Mat image,
                                   const yolo::BoxArray &box_array,
                                   bool draw_cls) {
  static const char *cocolabels[] = {
      "person",        "bicycle",      "car",
      "motorcycle",    "airplane",     "bus",
      "train",         "truck",        "boat",
      "traffic light", "fire hydrant", "stop sign",
      "parking meter", "bench",        "bird",
      "cat",           "dog",          "horse",
      "sheep",         "cow",          "elephant",
      "bear",          "zebra",        "giraffe",
      "backpack",      "umbrella",     "handbag",
      "tie",           "suitcase",     "frisbee",
      "skis",          "snowboard",    "sports ball",
      "kite",          "baseball bat", "baseball glove",
      "skateboard",    "surfboard",    "tennis racket",
      "bottle",        "wine glass",   "cup",
      "fork",          "knife",        "spoon",
      "bowl",          "banana",       "apple",
      "sandwich",      "orange",       "broccoli",
      "carrot",        "hot dog",      "pizza",
      "donut",         "cake",         "chair",
      "couch",         "potted plant", "bed",
      "dining table",  "toilet",       "tv",
      "laptop",        "mouse",        "remote",
      "keyboard",      "cell phone",   "microwave",
      "oven",          "toaster",      "sink",
      "refrigerator",  "book",         "clock",
      "vase",          "scissors",     "teddy bear",
      "hair drier",    "toothbrush"};
  for (const yolo::Box &obj : box_array) {
    if (obj.class_label != 0) {
      continue;
    }
    uint8_t b, g, r;
    tie(b, g, r) = iLogger::random_color(obj.class_label);
    cv::rectangle(image, cv::Point(obj.left, obj.top),
                  cv::Point(obj.right, obj.bottom), cv::Scalar(b, g, r), 5);
    if (draw_cls) {
      auto name = cocolabels[obj.class_label];
      auto caption = iLogger::format("%s %.2f", name, obj.confidence);
      int width = cv::getTextSize(caption, 0, 1, 2, nullptr).width + 10;
      cv::rectangle(image, cv::Point(obj.left - 3, obj.top - 33),
                    cv::Point(obj.left + width, obj.top), cv::Scalar(b, g, r),
                    -1);
      cv::putText(image, caption, cv::Point(obj.left, obj.top - 5), 0, 1,
                  cv::Scalar::all(0), 2, 16);
    }
  }
}

//    static cv::Rect get_rect(int w, int h, const float bbox[4]) {
//        int l, r, t, b;
//        float r_w = INPUT_W / (w * 1.0);
//        float r_h = INPUT_H / (h * 1.0);
//        if (r_h > r_w) {
//            l = bbox[0] - bbox[2] / 2.f;
//            r = bbox[0] + bbox[2] / 2.f;
//            t = bbox[1] - bbox[3] / 2.f - (INPUT_H - r_w * h) / 2;
//            b = bbox[1] + bbox[3] / 2.f - (INPUT_H - r_w * h) / 2;
//            l = l / r_w;
//            r = r / r_w;
//            t = t / r_w;
//            b = b / r_w;
//        } else {
//            l = bbox[0] - bbox[2] / 2.f - (INPUT_W - r_h * w) / 2;
//            r = bbox[0] + bbox[2] / 2.f - (INPUT_W - r_h * w) / 2;
//            t = bbox[1] - bbox[3] / 2.f;
//            b = bbox[1] + bbox[3] / 2.f;
//            l = l / r_h;
//            r = r / r_h;
//            t = t / r_h;
//            b = b / r_h;
//        }
//        return cv::Rect(l, t, r - l, b - t);
//    }
