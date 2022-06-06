#include <cheating_detection/cheating_detection.hpp>
#include <common/constant.h>
#include <common/curl_utils.hpp>
#include <tracker.h>

int smc::cht_det::cheatingDetection(std::atomic_ullong &endTime,
                                    video_io::VideoInput &videoInput,
                                    video_io::VideoOutput &videoOutput,
                                    cht_det::DataSaver &dataSaver,
                                    cht_det::Info &info) {
  if (not videoInput.isOpened() or not videoOutput.isOpened()) {
    INFOE("Cheating Detection start failed");
    return -1;
  }

  cv::Mat frame;
  Data data;     /**当前处理结果**/
  Data nextData; /**下一帧处理结果**/
  cv::Mat nextFrame;
  auto infer = createInfer();
  INFO("%llu endTime: %llu", iLogger::timestamp_now(),
       endTime.operator unsigned long long());
  const int delay = info.delay - smc::constant::VIDEO_CODE_DELAY;
  long long int curTime = iLogger::timestamp_now();
  const int skipFrameNum = 2;     /**抽帧数量**/
  int loopNum = 0;                /**循环帧号**/
  int saveNum = 1 % skipFrameNum; /**保存帧号**/
  while (videoInput.read(nextFrame) && curTime < endTime) {
    if (loopNum == 0)
      nextData = infer->commit(nextFrame);
    if (not frame.empty()) {
      if (loopNum == saveNum)
        dataSaver.save(data); // save data
      auto &box_array = data.bboxes.get();
      drawBboxes(frame, box_array);
      auto &poseActions = data.poseActions.get();
      auto &headPoses = data.headPoses.get();
      auto &cheatingState = data.cheatingStates.get();
      for (int i = 0; i < box_array.size(); ++i) {
        auto &box = box_array[i];
        auto &posAct = poseActions[i].get();
        auto &headPose = headPoses[i].get();
        drawKeypoints136(frame, posAct.pose); /**关键点**/
        //                                                             string
        //                                                             text(posAct.isPassing
        //                                                             == 0 ?
        //                                                             "no" : (
        //                                                                     posAct.isPassing > 0 ? "left"
        //                                                                                          : "right")); /**伸手**/
        //                                                             char
        //                                                             text[255];
        //                                                             sprintf(text,
        //                                                             "p=%.2f,y=%.2f,r=%.2f",
        //                                                                     headPose.eulerAngles[0],
        //                                                                     headPose.eulerAngles[1],
        //                                                                     headPose.eulerAngles[2]); /**头部角度**/
        std::string text(
            cls_act::CHEATING_TYPES_NAME[cheatingState[i].predState]);
        bool is_cheating = cheatingState[i].predState != 0;
        cv::putText(frame, text, cv::Point(box.left, box.top - 5), 0, 1,
                    is_cheating ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255),
                    2, 16);
      }
      for (auto &hp : data.headPoses.get()) {
        hp.get().drawFrameAxes(frame);
      }
      /**绘制拍摄时间**/
      time_t t = time(nullptr);
      char ch[64];
      strftime(ch, sizeof(ch), "%Y-%m-%d %H-%M-%S", localtime(&t));
      cv::putText(frame, ch, cv::Point(320, 240), cv::FONT_HERSHEY_PLAIN, 1,
                  cv::Scalar(255, 255, 255), 2);
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
    loopNum = (loopNum + 1) % skipFrameNum;
  }
  INFO("exiting cht_det task");
  return 0;
}
