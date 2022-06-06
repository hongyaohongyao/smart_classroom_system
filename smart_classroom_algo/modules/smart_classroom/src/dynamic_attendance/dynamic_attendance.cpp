#include <common/constant.h>
#include <common/json_utils.hpp>
#include <dynamic_attendance/dynamic_attendance.hpp>

using namespace std;
using namespace smc;

static cv::Mat_<float> json2feature_map(cJSON *j_array) {
  if (not cJSON_AS4CPP_IsArray(j_array)) {
    return {};
  }
  int out_size = cJSON_AS4CPP_GetArraySize(j_array);
  if (out_size <= 0) {
    return {};
  }
  cJSON *inner_array = cJSON_AS4CPP_GetArrayItem(j_array, 0);
  int inner_size = cJSON_AS4CPP_GetArraySize(inner_array);
  cv::Mat_<float> features(out_size, inner_size);
  for (int j = 0; j < inner_size; ++j) {
    features[0][j] = (float)cJSON_AS4CPP_GetNumberValue(
        cJSON_AS4CPP_GetArrayItem(inner_array, j));
  }
  for (int i = 1; i < out_size; i++) {
    inner_array = cJSON_AS4CPP_GetArrayItem(j_array, i);
    if (cJSON_AS4CPP_GetArraySize(inner_array) != inner_size) {
      INFOE("wrong features map");
      return {};
    }
    for (int j = 0; j < inner_size; ++j) {
      features[i][j] = (float)cJSON_AS4CPP_GetNumberValue(
          cJSON_AS4CPP_GetArrayItem(inner_array, j));
    }
  }
  return features;
}

static vector<dyn_att::MemInfo> json2mem_vec(cJSON *j_array) {
  if (not cJSON_AS4CPP_IsArray(j_array)) {
    return {};
  }
  int j_size = cJSON_AS4CPP_GetArraySize(j_array);
  if (j_size <= 0) {
    return {};
  }
  vector<dyn_att::MemInfo> result;
  for (int i = 0; i < j_size; i++) {
    auto *pItem = cJSON_AS4CPP_GetArrayItem(j_array, i);
    if (pItem != nullptr) {
      auto *j_name = cJSON_AS4CPP_GetObjectItem(pItem, "name");
      auto *j_mem_id = cJSON_AS4CPP_GetObjectItem(pItem, "memId");
      if (cJSON_AS4CPP_IsString(j_mem_id) && cJSON_AS4CPP_IsString(j_name)) {
        result.emplace_back(cJSON_AS4CPP_GetStringValue(j_name),
                            cJSON_AS4CPP_GetStringValue(j_mem_id));
      } else {
        return {};
      }

    } else {
      return {};
    }
  }
  return result;
}

int smc::dyn_att::dynamicAttendance(std::atomic_ullong &endTime,
                                    video_io::VideoInput &videoInput,
                                    video_io::VideoOutput &videoOutput,
                                    dyn_att::DataSaver &dataSaver,
                                    dyn_att::Info &info) {
  if (not videoInput.isOpened() or not videoOutput.isOpened()) {
    INFOE("Cheating Detection start failed");
    return -1;
  }

  cv::Mat frame;
  Data data;     /**当前处理结果**/
  Data nextData; /**下一帧处理结果**/
  cv::Mat nextFrame;
  auto &memInfos = info.memInfos;
  auto infer = createInfer(info);
  char text_buf[1024];
  char ch[64];
  auto color_black = cv::Scalar::all(0);
  auto color_red = cv::Scalar(0, 0, 255);
  INFO("%llu end_time: %llu", iLogger::timestamp_now(),
       endTime.operator unsigned long long());
  const int delay = info.delay - smc::constant::VIDEO_CODE_DELAY;
  long long int curTime = iLogger::timestamp_now();
  const int skipNum = 2;     /**抽帧数量**/
  int skipCount = 0;         /**循环帧号**/
  int saveNum = 1 % skipNum; /**保存帧号**/
  while (videoInput.read(nextFrame) && curTime < endTime) {
    float unknow_thres = info.unknownThres;
    if (skipCount == 0)
      nextData = infer->commit(nextFrame);
    if (not frame.empty()) {
      if (skipCount == saveNum)
        dataSaver.save(data); // save data
      auto &box_array = data.bboxes.get();
      auto &scores = data.scores.get();
      for (int i = 0; i < box_array.size(); ++i) {
        auto &box = box_array[i];
        auto &score = scores[i];
        cv::rectangle(frame, cv::Point((int)box.left, (int)box.top),
                      cv::Point((int)box.right, (int)box.bottom), color_red, 2);
        if (score.confidence > unknow_thres) {
          sprintf(text_buf, "%s %.2f%", memInfos[score.idx].name.c_str(),
                  score.confidence * 100);
        } else {
          sprintf(text_buf, "%s", "unknown");
        }

        cv::putText(frame, text_buf, cv::Point((int)box.left, (int)box.top - 5),
                    0, 1, color_red, 2, 16);
      }
      /**绘制拍摄时间**/
      time_t t = time(nullptr);
      strftime(ch, sizeof(ch), "%Y-%m-%d %H-%M-%S", localtime(&t));
      cv::putText(frame, ch, cv::Point(frame.cols / 2, frame.rows / 2),
                  cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255), 2);
      /**输出视频帧**/
      videoOutput.write(frame);
      int rightDelay =
          std::max(0, delay - (int)(iLogger::timestamp_now() - curTime));
      // make delay
      if (cv::waitKey(rightDelay) == 'p')
        break;
    }
    curTime = iLogger::timestamp_now(); // update curTime
    nextFrame.copyTo(frame);
    data = nextData;
    skipCount = (skipCount + 1) % skipNum;
  }
  return 0;
}

cv::Mat smc::dyn_att::string2featureMap(const std::string &str) {
  cJSON *j_array = cJSON_AS4CPP_Parse(str.c_str());
  if (j_array == nullptr) {
    return {};
  }
  cv::Mat result = json2feature_map(j_array);
  cJSON_AS4CPP_Delete(j_array);
  return result;
}

std::shared_ptr<dyn_att::Info>
smc::dyn_att::generateInfo(const std::string &jsonStr) {
  cJSON *att_info = cJSON_AS4CPP_Parse(jsonStr.c_str());
  if (att_info == nullptr) {
    return nullptr;
  }
  cJSON *memInfos = cJSON_AS4CPP_GetObjectItem(att_info, "memInfos");
  cJSON *featureMap = cJSON_AS4CPP_GetObjectItem(att_info, "featureMap");

  auto &&fm = json2feature_map(featureMap);
  if (fm.empty())
    return nullptr;
  auto &&mi = json2mem_vec(memInfos);
  if (mi.size() != fm.rows) {
    return nullptr;
  }

  auto info = make_shared<dyn_att::Info>();
  info->featureMap = fm;
  info->memInfos = mi;
  cJSON_AS4CPP_Delete(att_info);

  return info;
}
