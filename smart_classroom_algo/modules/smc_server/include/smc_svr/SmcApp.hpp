
#ifndef SMART_CLASSROOM_CPP_SMCAPP_HPP
#define SMART_CLASSROOM_CPP_SMCAPP_HPP

#include <atomic>
#include <common/ilogger.hpp>
#include <drogon/HttpController.h>
#include <map>
#include <mutex>
#include <thread>

using namespace drogon;
namespace smc {
struct TaskInfo {
  std::shared_ptr<std::thread> worker;
  std::atomic_ullong end_time{};
  std::shared_ptr<void> data{};
};

class SmcApp : public HttpController<SmcApp> {
  const int max_num = SMC_MAX_RESOURCE;
  std::mutex counter_mtx;
  int counter = SMC_MAX_RESOURCE;
  std::mutex threads_mtx;
  std::map<std::string, TaskInfo> threads_;

public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(SmcApp::isAlive, "/is_alive", Get);
  ADD_METHOD_TO(SmcApp::resetEndTime, "/reset_end_time", Get);
  ADD_METHOD_TO(SmcApp::hasTask, "/has_task", Get);
  ADD_METHOD_TO(SmcApp::stopTask, "/stop_task", Delete);
  ADD_METHOD_TO(SmcApp::numOfTasks, "/num_of_tasks", Get);
  ADD_METHOD_TO(SmcApp::regFace, "/reg_face", Get);
  ADD_METHOD_TO(SmcApp::chtDet, "/cht_det", Post);
  ADD_METHOD_TO(SmcApp::dynAtt, "/dyn_att", Post);
  METHOD_LIST_END

  SmcApp() { INFO("start smart classroom application"); }

protected:
  static void isAlive(const HttpRequestPtr &,
                      std::function<void(const HttpResponsePtr &)> &&callback);

  void resetEndTime(const HttpRequestPtr &,
                    std::function<void(const HttpResponsePtr &)> &&callback);

  void hasTask(const HttpRequestPtr &,
               std::function<void(const HttpResponsePtr &)> &&callback);

  void stopTask(const HttpRequestPtr &,
                std::function<void(const HttpResponsePtr &)> &&callback);

  void numOfTasks(const HttpRequestPtr &,
                  std::function<void(const HttpResponsePtr &)> &&callback);

  static void regFace(const HttpRequestPtr &,
                      std::function<void(const HttpResponsePtr &)> &&callback);

  void chtDet(const HttpRequestPtr &,
              std::function<void(const HttpResponsePtr &)> &&callback);

  void dynAtt(const HttpRequestPtr &,
              std::function<void(const HttpResponsePtr &)> &&callback);
};

} // namespace smc
#endif // SMART_CLASSROOM_CPP_SMCAPP_HPP
