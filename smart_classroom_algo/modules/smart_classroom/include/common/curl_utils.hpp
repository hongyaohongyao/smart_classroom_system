
#ifndef SMART_CLASSROOM_CPP_CURL_UTILS_HPP
#define SMART_CLASSROOM_CPP_CURL_UTILS_HPP

#include <curl/curl.h>
#include <opencv2/opencv.hpp>
#include <cstring>

namespace smc::curl_utils {
#define DEFAULT_USER_PWD   "smc:123456"

    cv::Mat httpDownloadImg(const char *url);

    void postUpLoad(const cv::Mat &img, const char *url);

    void ftpUpload(const cv::Mat &img, const char *url, const char *user_pwd = DEFAULT_USER_PWD, int quality = 70);

    void ftpUpload(const cv::Mat &img, const char *url, const std::vector<int> &vecCompression_params,
                   const char *user_pwd = DEFAULT_USER_PWD);

    void ftpUpload(const std::string &contents, const char *url, const char *user_pwd = DEFAULT_USER_PWD);

    std::string ftpDownload(const char *url, const char *user_pwd = DEFAULT_USER_PWD);
}
#endif //SMART_CLASSROOM_CPP_CURL_UTILS_HPP
