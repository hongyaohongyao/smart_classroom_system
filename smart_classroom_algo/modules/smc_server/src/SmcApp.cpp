#include <smc_svr/SmcApp.hpp>
#include <common/ilogger.hpp>

namespace smc {
    using namespace std;

    void SmcApp::isAlive(const HttpRequestPtr &, std::function<void(const HttpResponsePtr &)> &&callback) {

        auto resp = HttpResponse::newHttpResponse();
        resp->setBody("ok");
        callback(resp);
    }

    void
    SmcApp::resetEndTime(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) {
        auto res = HttpResponse::newHttpResponse();
        auto &params = req->getParameters();
        auto task_name_it = params.find("task_name");
        auto end_time_it = params.find("end_time");
        if (task_name_it != params.end() && end_time_it != params.end()) {
            string task_name = task_name_it->second;
            size_t end_time = stoull(end_time_it->second);

            {
                lock_guard<mutex> mtx_lock(threads_mtx);
                auto it = threads_.find(task_name);
                if (it != threads_.end()) {
                    auto &task_info = threads_[task_name];
                    task_info.end_time = end_time;
                    INFO("reset end time of task %s", task_name.c_str());
                } else {
                    res->setBody("no");
                    callback(res);
                    return;
                }
                if (end_time < iLogger::timestamp_now()) {
                    INFOW("stopped task by reset end time %s", task_name.c_str());
                    res->setBody("no");
                    callback(res);
                    return;
                }
            }
        } else {
            res->setBody("need param <task_name> <end_time>");
            callback(res);
            return;
        }
        res->setBody("ok");
        callback(res);
    }

    void SmcApp::hasTask(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) {
        auto res = HttpResponse::newHttpResponse();
        auto &params = req->getParameters();
        auto task_name_it = params.find("task_name");
        if (task_name_it != params.end()) {
            string task_name = task_name_it->second;
            {
                lock_guard<mutex> mtx_lock(threads_mtx);
                auto it = threads_.find(task_name);
                if (it != threads_.end()) {
                    res->setBody("ok");
                } else {
                    res->setBody("no");
                }
            }
        } else {
            res->setBody("need param <task_name>");
        }
        callback(res);
    }

    void SmcApp::stopTask(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) {
        auto res = HttpResponse::newHttpResponse();
        auto &params = req->getParameters();
        auto task_name_it = params.find("task_name");
        if (task_name_it != params.end()) {
            string task_name = task_name_it->second;
            INFO("stopping a task %s", task_name.c_str());
            {
                lock_guard<mutex> mtx_lock(threads_mtx);
                auto it = threads_.find(task_name);
                if (it != threads_.end()) {
                    auto &task_info = threads_[task_name];
                    task_info.end_time = 0;
                    INFO("stopped task %s", task_name.c_str());
                }
            }
        } else {
            res->setBody("need param <task_name>");
            callback(res);
            return;
        }
        res->setBody("ok");
        callback(res);
    }

    void SmcApp::numOfTasks(const HttpRequestPtr &, std::function<void(const HttpResponsePtr &)> &&callback) {
        auto res = HttpResponse::newHttpResponse();
        lock_guard<mutex> mtx_lock(threads_mtx);
        res->setBody(to_string(threads_.size()));
        callback(res);
    }
}


