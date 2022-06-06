

#ifndef SMART_CLASSROOM_CPP_JSON_UTILS_HPP
#define SMART_CLASSROOM_CPP_JSON_UTILS_HPP

#include "aws/core/external/cjson/cJSON.h"
#include <fstream>
#include "common/curl_utils.hpp"

namespace smc {
    namespace json_utils {

        std::string json2uf_str(cJSON *obj) ;

        void saveJson(const std::string &url, cJSON *jsonObj) ;

        void ftpSaveJson(const std::string &url, cJSON *jsonObj, const char *userPwd) ;
    };
}
#endif //SMART_CLASSROOM_CPP_JSON_UTILS_HPP
