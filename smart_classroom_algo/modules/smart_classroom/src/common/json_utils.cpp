#include <common/json_utils.hpp>

using namespace std;

std::string smc::json_utils::json2uf_str(cJSON *obj) {
    char *res = cJSON_AS4CPP_PrintUnformatted(obj);
    std::string &&result = res;
    return result;
}

void smc::json_utils::saveJson(const std::string &url, cJSON *jsonObj) {
    char *results;
    std::ofstream outfile(url);
    results = cJSON_AS4CPP_PrintUnformatted(jsonObj); // get json string

    outfile << results;

    outfile.close(); // close file
    cJSON_AS4CPP_free(results); // free print result from cJson
}

void smc::json_utils::ftpSaveJson(const std::string &url, cJSON *jsonObj, const char *userPwd) {
    char *results;
    results = cJSON_AS4CPP_PrintUnformatted(jsonObj); // get json string
    std::string out_str(results);
    curl_utils::ftpUpload(out_str, url.c_str(), userPwd);
    cJSON_AS4CPP_free(results); // free print result from cJson
}