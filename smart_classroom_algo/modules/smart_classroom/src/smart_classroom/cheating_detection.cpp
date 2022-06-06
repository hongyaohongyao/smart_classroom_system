#include <cheating_detection/data_saver.hpp>
#include <smart_classroom.hpp>

using namespace std;

int smc::smc_app::cheatingDetection(atomic_ullong &end_time,
                                    const string &source,
                                    const string &taskName, int input_type,
                                    int output_type, int file_sys_type,
                                    const string &rtmpServer,
                                    const string &fileSysDir) {
  char strBuf[1024];
  cht_det::Info info;
  /**设置视频输入**/
  shared_ptr<video_io::VideoInput> video_input;
  shared_ptr<video_io::VideoOutput> video_output;
  shared_ptr<cht_det::DataSaver> data_saver;
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
    fps = 15;
    video_input = make_shared<video_io::OpencvVideoCaptureWrapper>(
        move(cam)); /**调用电脑摄像头**/
  }
  info.delay = 1000 / fps;
  INFO("video source ok: %s type: %s", source.c_str(),
       to_string(input_type).c_str());
  /**设置视频输出**/
  if (output_type == OUTPUT_TYPE_RTMP) {
    snprintf(strBuf, 1024, "rtmp://%s/demo/%s", rtmpServer.c_str(),
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
    snprintf(strBuf, 1024, "%s/snapshots/%s", fileSysDir.c_str(),
             taskName.c_str());
    data_saver = make_shared<cht_det::LocalDataSaver>(strBuf);
  } else if (file_sys_type == FILE_SYS_TYPE_FTP) {
    data_saver = make_shared<cht_det::FtpDataSaver>(
        fileSysDir + ":21", "smc", "123456", "/snapshots/" + taskName);
  } else if (file_sys_type == FILE_SYS_TYPE_MINIO) {
    data_saver = make_shared<cht_det::MinioDataSaver>(
        MINIO_HOST, MINIO_ACCESS_KEY, MINIO_SECRET_KEY, "snapshots", taskName);
  } else {
    INFOE("unknown file sys type");
    return -1;
  }
  INFO("file system output ok, type: %s", to_string(file_sys_type).c_str());

  INFO("created capture and writer");
  return cht_det::cheatingDetection(end_time, *video_input, *video_output,
                                    *data_saver, info);
}