#include <thread>
#include <mutex>
#include <map>
#include <smart_classroom.hpp>
#include <extra/httplib.h>


using namespace std;
using namespace httplib;

struct ServerMainInfo {

};

struct TaskInfo {
    shared_ptr<thread> worker;
    atomic_ullong end_time{};
    shared_ptr<void> data{};
};

int smc::smc_app::serverMain(int argc, char **argv) {
    /*************************************\
         public data
    \*************************************/

    const int max_num = SMC_MAX_RESOURCE;
    mutex counter_mtx;
    int counter = SMC_MAX_RESOURCE;
    mutex threads_mtx;

    map<string, TaskInfo> threads_;


    Server svr;
    /*************************************\
             register router
    \*************************************/

    /**** for running info ****/

    svr.Get("/is_alive", [](const Request &req, Response &res) {
        res.set_content("ok", "text/plain");
    });

    svr.Get("/reset_end_time", [&](const Request &req, Response &res) {
        if (req.has_param("task_name") && req.has_param("end_time")) {
            string task_name = req.get_param_value("task_name");
            size_t end_time = stoull(req.get_param_value("end_time"));

            INFO("reset end time of a task %s", task_name.c_str());
            {
                lock_guard<mutex> mtx_lock(threads_mtx);
                auto it = threads_.find(task_name);
                if (it != threads_.end()) {
                    auto &task_info = threads_[task_name];
                    task_info.end_time = end_time;
                    INFO("reset end time of task %s", task_name.c_str());
                } else {
                    res.set_content("no", "text/plain");
                    return;
                }
                if (end_time < iLogger::timestamp_now()) {
                    INFOW("stopped task by reset end time %s", task_name.c_str());
                    res.set_content("no", "text/plain");
                    return;
                }
            }
        } else {
            res.set_content("need param <task_name> <end_time>", "text/plain");
            return;
        }
        res.set_content("ok", "text/plain");
    });

    svr.Get("/has_task", [&](const Request &req, Response &res) {
        if (req.has_param("task_name")) {
            string task_name = req.get_param_value("task_name");
            {
                lock_guard<mutex> mtx_lock(threads_mtx);
                auto it = threads_.find(task_name);
                if (it != threads_.end()) {
                    res.set_content("ok", "text/plain");
                } else {
                    res.set_content("no", "text/plain");
                }
            }
        } else {
            res.set_content("need param task_name", "text/plain");
        }
    });

    svr.Delete("/stop_task", [&](const Request &req, Response &res) {
        if (req.has_param("task_name")) {
            string task_name = req.get_param_value("task_name");
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
            res.set_content("need param <task_name>", "text/plain");
            return;
        }
        res.set_content("ok", "text/plain");
    });

    svr.Get("/num_of/last", [&](const Request &req, Response &res) {
        lock_guard<mutex> mtx_lock(counter_mtx);
        res.set_content(to_string(counter), "text/plain");
    });

    svr.Get("/num_of/running", [&](const Request &req, Response &res) {
        lock_guard<mutex> mtx_lock(threads_mtx);
        res.set_content(to_string(threads_.size()), "text/plain");
    });

    /**** register face ****/

    svr.Get("/reg_face", [&](const Request &req, Response &res) {
        if (not req.has_param("url")) {
            res.set_content("need param <url>", "text/plain");
            return;
        }
        string &&result = smc::dyn_att::registerFace(req.get_param_value("url"));
        res.set_content(result, "text/plain");
    });

    /**** cheating detection ****/

    svr.Post("/cht_det", [&](const Request &req, Response &res) {
        INFO("requesting a cht det task");
        string src;
        string task_name;
        int in_type = smc::smc_app::INPUT_TYPE_LOCAL;
        int out_type = smc::smc_app::OUTPUT_TYPE_RTMP;
        int file_sys_type = smc::smc_app::FILE_SYS_TYPE_MINIO;
        string dst = SMC_RTMP_SERVER;
        string file_sys = SMC_SERVER_HOST;
        size_t end_time = ULLONG_MAX;

        if (req.has_file("src")) {
            src = req.get_file_value("src").content;
        } else {
            res.set_content("need param <src> as video source.", "text/plain");
            return;
        }
        if (req.has_file("task_name")) {
            task_name = req.get_file_value("task_name").content;
        } else {
            res.set_content("need param <task_name> as task name", "text/plain");
            return;
        }
        if (req.has_file("in_type")) {
            string tmp_str = req.get_file_value("in_type").content;
            in_type = stoi(tmp_str);
        }
        if (req.has_file("out_type")) {
            string tmp_str = req.get_file_value("out_type").content;
            out_type = stoi(tmp_str);
        }
        if (req.has_file("file_sys_type")) {
            string tmp_str = req.get_file_value("file_sys_type").content;
            file_sys_type = stoi(tmp_str);
        }
        if (req.has_file("dst")) {
            dst = req.get_file_value("dst").content;
        }
        if (req.has_file("file_sys")) {
            file_sys = req.get_file_value("file_sys").content;
        }
        if (req.has_file("end_time")) {
            end_time = stoull(req.get_file_value("end_time").content);
        }

        {
            lock_guard<mutex> mtx_lock(counter_mtx);
            if (counter <= 0) {
                res.set_content("no enough processing resource.", "text/plain");
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
                res.set_content("task has exists", "text/plain");
                return;
            }
            task_info = &threads_[task_name];
        }
        task_info->worker = make_shared<thread>([=,
                                                        &counter_mtx,
                                                        &counter,
                                                        &threads_mtx,
                                                        &threads_]() {
            task_info->end_time = end_time;
            smc::smc_app::cheatingDetection(task_info->end_time,
                                            src,
                                            task_name,
                                            in_type,
                                            out_type,
                                            file_sys_type,
                                            dst,
                                            file_sys);
            INFO("removing cht_det task");
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
            INFO("successfully exit cht_det task");
        });
        res.set_content("ok", "text/plain");
    });

    /**** dynamic attendance ****/

    svr.Post("/dyn_att", [&](const Request &req, Response &res) {
        INFO("requesting a dyn att task");
        string src;
        string task_name;
        int in_type = smc::smc_app::INPUT_TYPE_LOCAL;
        int out_type = smc::smc_app::OUTPUT_TYPE_RTMP;
        int file_sys_type = smc::smc_app::FILE_SYS_TYPE_MINIO;
        string dst = SMC_RTMP_SERVER;
        string file_sys = SMC_SERVER_HOST;
        size_t end_time = iLogger::timestamp_now() + 120000;// 1 min default

        if (req.has_file("src")) {
            src = req.get_file_value("src").content;
        } else {
            res.set_content("need form value <src> as video source.", "text/plain");
            return;
        }
        if (req.has_file("task_name")) {
            task_name = req.get_file_value("task_name").content;
        } else {
            res.set_content("need form value <task_name> as task name", "text/plain");
            return;
        }
        if (req.has_file("in_type")) {
            string tmp_str = req.get_file_value("in_type").content;
            in_type = stoi(tmp_str);
        }

        shared_ptr<dyn_att::Info> info;
        if (req.has_file("att_info")) {
            auto info_str = req.get_file_value("att_info").content;
            info = dyn_att::generateInfo(info_str);
            if (info == nullptr) {
                res.set_content(
                        "wrong format for att_info {names:[...],featureMap:[[...],...]}",
                        "text/plain"
                );
                return;
            }
        } else {
            res.set_content("need form value <att_info> as info of task", "text/plain");
            return;
        }
//        if (req.has_file("dst")) {
//            dst = req.get_file_value("dst");
//        }
//        if (req.has_file("end_time")) {
//            end_time = stoull(req.get_file_value("end_time"));
//        }

        {
            lock_guard<mutex> mtx_lock(counter_mtx);
            if (counter <= 0) {
                res.set_content("no enough processing resource.", "text/plain");
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
                res.set_content("task has exists", "text/plain");
                return;
            }
            task_info = &threads_[task_name];
        }
        task_info->worker = make_shared<thread>([=,
                                                        &counter_mtx,
                                                        &counter,
                                                        &threads_mtx,
                                                        &threads_]() {
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
        res.set_content("ok", "text/plain");
    });

    /*************************************\
     start server
    \*************************************/

    INFO((string("listen to ") + SMC_SERVER_HOST + ":" + to_string(SMC_SERVER_PORT)).c_str());
    svr.listen(SMC_SERVER_HOST, SMC_SERVER_PORT);
    return 0;
}
