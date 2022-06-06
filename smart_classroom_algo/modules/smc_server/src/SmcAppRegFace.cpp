#include <smc_svr/SmcApp.hpp>
#include <common/ilogger.hpp>
#include <dynamic_attendance/dynamic_attendance.hpp>

namespace smc {
    using namespace std;

    void SmcApp::regFace(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) {
        auto res = HttpResponse::newHttpResponse();
        auto &params = req->getParameters();
        auto url_it = params.find("url");
        if (url_it != params.end()) {
            string &&result = smc::dyn_att::registerFace(url_it->second);
            res->setBody(result);
        } else {
            res->setBody("need param <url>");
        }
        callback(res);
        return;
    }
}