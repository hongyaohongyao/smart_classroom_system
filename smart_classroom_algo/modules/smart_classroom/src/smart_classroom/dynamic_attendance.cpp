#include <common/models_utils.hpp>
#include <dynamic_attendance/data_saver.hpp>
#include <smart_classroom.hpp>

using namespace std;

int smc::smc_app::dynamicAttendance(std::atomic_ullong &end_time,
                                    dyn_att::Info &info,
                                    const std::string &source,
                                    const std::string &taskName, int input_type,
                                    int output_type, int file_sys_type,
                                    const std::string &dstUrl,
                                    const std::string &fileSysDir) {
  if (info.memInfos.empty() || info.memInfos.size() != info.featureMap.rows) {
    INFOE("empty face library");
    return 1;
  }
  char strBuf[1024];
  /**设置视频输入**/
  shared_ptr<video_io::VideoInput> video_input;
  shared_ptr<video_io::VideoOutput> video_output;
  shared_ptr<dyn_att::DataSaver> data_saver;
  int width = 640, height = 480, fps = 15;
  if (input_type == INPUT_TYPE_LOCAL) {
    snprintf(strBuf, 1024, "%s/smc_videos/%s", SMC_FILE_SYS_DIR,
             source.c_str());
    INFO(strBuf);
    cv::VideoCapture cam(strBuf);
    width = (int)cam.get(cv::CAP_PROP_FRAME_WIDTH);
    height = (int)cam.get(cv::CAP_PROP_FRAME_HEIGHT);
    fps = (int)cam.get(cv::CAP_PROP_FPS);
    if (fps <= 0)
      fps = 15;
    video_input = make_shared<video_io::OpencvVideoCaptureWrapper>(
        move(cam), SMC_LOOP_VIDEO);
  } else if (input_type == INPUT_TYPE_WEB) {
    INFO(source.c_str());
    cv::VideoCapture cam(source);
    width = (int)cam.get(cv::CAP_PROP_FRAME_WIDTH);
    height = (int)cam.get(cv::CAP_PROP_FRAME_HEIGHT);
    fps = (int)cam.get(cv::CAP_PROP_FPS);
    if (fps <= 0)
      fps = 15;
    video_input = make_shared<video_io::OpencvVideoCaptureWrapper>(
        move(cam), SMC_LOOP_VIDEO);
  } else {
    cv::VideoCapture cam(cv::CAP_ANY);
    width = (int)cam.get(cv::CAP_PROP_FRAME_WIDTH);
    height = (int)cam.get(cv::CAP_PROP_FRAME_HEIGHT);
    fps = (int)cam.get(cv::CAP_PROP_FPS);
    if (fps <= 0)
      fps = 15;
    video_input = make_shared<video_io::OpencvVideoCaptureWrapper>(
        move(cam)); /**调用电脑摄像头**/
  }
  info.delay = 1000 / fps;
  INFO("video source ok: %s type: %s", source.c_str(),
       to_string(input_type).c_str());
  /**设置视频输出**/
  if (output_type == OUTPUT_TYPE_RTMP) {
    snprintf(strBuf, 1024, "rtmp://%s/demo/%s", dstUrl.c_str(),
             taskName.c_str());
    video_output =
        make_shared<video_io::RtmpVideoOutput>(strBuf, width, height, fps);
  } else {
    video_output =
        make_shared<video_io::ShowVideoOutput>("haha"); /**窗口输出画面**/
  }
  INFO("video output ok, type: %s", to_string(output_type).c_str());

  /**设置检测信息保存位置**/
  if (file_sys_type == FILE_SYS_TYPE_LOCAL) {
    snprintf(strBuf, 1024, "%s/local/snapshots/%s", fileSysDir.c_str(),
             taskName.c_str());
    data_saver = make_shared<dyn_att::LocalDataSaver>(strBuf, info);
  } else if (file_sys_type == FILE_SYS_TYPE_MINIO) {
    data_saver = make_shared<dyn_att::MinioDataSaver>(
        info, MINIO_HOST, MINIO_ACCESS_KEY, MINIO_SECRET_KEY, "snapshots",
        taskName);
  } else {
    INFOE("unknown file sys type");
    return -1;
  }
  INFO("file system output ok, type: %s", to_string(file_sys_type).c_str());

  INFO("created capture and writer");
  return dyn_att::dynamicAttendance(end_time, *video_input, *video_output,
                                    *data_saver, info);
}

int smc::smc_app::testDynamicAttendance() {
  auto scrfd = models_utils::getScrfd();
  auto arcface = models_utils::getArcface();
  atomic_ullong endTime(iLogger::timestamp_now() + 25000);
  dyn_att::Info info;
  string baseUrl = "./facebank";
  auto files = iLogger::find_files(baseUrl, "*", true);
  cv::Mat_<float> featuresMap(0, 512);
  vector<dyn_att::MemInfo> memInfos;
  int idx = 0;
  for (auto &file : files) {
    memInfos.emplace_back(file.substr(baseUrl.size() + 1), to_string(idx++));
    auto img = cv::imread(file + "/face.jpg");
    auto faceBoxes = scrfd->commit(img).get();
    if (faceBoxes.empty()) {
      INFOE("wrong img %d");
      return 1;
    }

    int max_face_index =
        (int)(std::max_element(
                  faceBoxes.begin(), faceBoxes.end(),
                  [](face_detector::Box &face1, face_detector::Box &face2) {
                    return face1.area() < face2.area();
                  }) -
              faceBoxes.begin());

    auto crop = scrfd::crop_face_and_landmark(img, faceBoxes[max_face_index]);
    auto res = arcface->commit(crop).get();
    featuresMap.push_back(res);
  }
  info.memInfos = memInfos;
  info.featureMap = featuresMap;

  INFO("face bank size: %d feature %d", featuresMap.rows, featuresMap.cols);

  return dynamicAttendance(endTime, info);
}