#include <smc_svr/SmcApp.hpp>
#include <common/ilogger.hpp>
#include <smart_classroom.hpp>

namespace smc {
    using namespace std;

    void SmcApp::dynAtt(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) {
        INFO("requesting a dyn att task");

        string src;
        string task_name;
        int in_type = smc::smc_app::INPUT_TYPE_LOCAL;
        int out_type = smc::smc_app::OUTPUT_TYPE_RTMP;
        int file_sys_type = smc::smc_app::FILE_SYS_TYPE_MINIO;
        string dst = SMC_RTMP_SERVER;
        string file_sys = SMC_SERVER_HOST;
        size_t end_time = iLogger::timestamp_now() + 90000;// 1 min default
        auto res = HttpResponse::newHttpResponse();

        auto params = req->getParameters();

//        string body;
//        body = req->getBody();
//        iLogger::save_file("./body.txt", body, true);

        auto src_it = params.find("src");
        if (src_it != params.end()) {
            src = src_it->second;
        } else {
            res->setBody("need form value <src> as video source.");
            callback(res);
            return;
        }

        auto task_name_it = params.find("task_name");
        if (task_name_it != params.end()) {
            task_name = task_name_it->second;
        } else {
            res->setBody("need form value <task_name> as task name");
            callback(res);
            return;
        }

        auto in_type_it = params.find("in_type");
        if (in_type_it != params.end()) {
            string tmp_str = in_type_it->second;
            in_type = stoi(tmp_str);
        }

        auto end_time_it = params.find("end_time");
        if (end_time_it != params.end()) {
            end_time = stoull(end_time_it->second);
        }

        shared_ptr<dyn_att::Info> info;
        auto att_info_it = params.find("att_info");
        if (att_info_it != params.end()) {
            auto info_str = att_info_it->second;
            info = dyn_att::generateInfo(info_str);
            if (info == nullptr) {
                res->setBody("wrong format for att_info {names:[...],featureMap:[[...],...]}");
                callback(res);
                return;
            }
        } else {
            res->setBody("need form value <att_info> as info of task");
            callback(res);
            return;
        }

        {
            lock_guard<mutex> mtx_lock(counter_mtx);
            if (counter <= 0) {
                res->setBody("no enough processing resource.");
                callback(res);
                return;
            } else {
                counter--;
            }
        }
        TaskInfo *task_info;
        {
            lock_guard<mutex> mtx_lock(threads_mtx);
            if (threads_.find(task_name) != threads_.end()) {
                {
                    lock_guard<mutex> mtx_lock(counter_mtx);
                    counter++;
                }
                res->setBody("task has exists.");
                callback(res);
                return;
            }
            task_info = &threads_[task_name];
        }
        task_info->worker = make_shared<thread>([=]() {
            task_info->end_time = end_time;
            task_info->data = info;
            smc::smc_app::dynamicAttendance(task_info->end_time,
                                            *info,
                                            src,
                                            task_name,
                                            in_type,
                                            out_type,
                                            file_sys_type,
                                            dst,
                                            file_sys);
            {
                lock_guard<mutex> mtx_lock(threads_mtx);
                auto it = threads_.find(task_name);
                if (it != threads_.end()) {
                    it->second.worker->detach();
                    threads_.erase(it);
                }
            }
            {
                lock_guard<mutex> mtx_lock(counter_mtx);
                counter++;
            }
            INFO("successfully exit dyn att task");
        });
        res->setBody("ok");
        callback(res);
    }
}