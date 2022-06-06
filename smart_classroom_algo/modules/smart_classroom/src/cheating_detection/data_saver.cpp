#include <cheating_detection/data_saver.hpp>
#include <common/curl_utils.hpp>
#include <fstream>
#include <sstream>
#include <utility>

using namespace std;

smc::cht_det::LocalDataSaver::LocalDataSaver(const string &baseUrl) : BaseDataSaver(baseUrl) {
    if (iLogger::mkdirs(this->baseUrl)) {
        INFO("create directory success");
    }
    initOrResume();
}

string smc::cht_det::LocalDataSaver::loadFile(const string &url) {
    ifstream infile(url);
    if (infile) {
        /**读取全部字符串**/
        ostringstream tmp;
        tmp << infile.rdbuf();
        return tmp.str();
    } else {
        return "";
    }
}

void smc::cht_det::LocalDataSaver::saveFile(const string &content, const string &url) {
    ofstream outfile(url);
    outfile << content;
    outfile.close(); // close file
}

void smc::cht_det::LocalDataSaver::saveImage(const cv::Mat &image, const string &url,
                                             const vector<int> &vecCompression_params) {
    cv::imwrite(url, image, vecCompression_params);
}


smc::cht_det::FtpDataSaver::FtpDataSaver(const string &host,
                                         const string &user,
                                         const string &pw,
                                         const string &baseUrl) : BaseDataSaver("ftp://" + host + "/" + baseUrl) {
    this->userPwd = user + ":" + pw;
    initOrResume();
}

string smc::cht_det::FtpDataSaver::loadFile(const string &url) {
    return curl_utils::ftpDownload(url.c_str(), userPwd.c_str());
}

void smc::cht_det::FtpDataSaver::saveFile(const string &content, const string &url) {
    curl_utils::ftpUpload(content, url.c_str(), userPwd.c_str());
}

void smc::cht_det::FtpDataSaver::saveImage(const cv::Mat &image, const string &url,
                                           const vector<int> &vecCompression_params) {
    curl_utils::ftpUpload(image, url.c_str(), vecCompression_params, userPwd.c_str());
}

smc::cht_det::MinioDataSaver::MinioDataSaver(const string &endPoint,
                                             const string &accessKey,
                                             const string &secretKey,
                                             const string &bucketName,
                                             const string &baseUrl) : BaseDataSaver(baseUrl),
                                                                      endPoint(endPoint),
                                                                      accessKey(accessKey),
                                                                      secretKey(secretKey),
                                                                      bucketName(bucketName) {
    client_ = make_shared<s3_utils::MinioClient>(endPoint, accessKey, secretKey);
    initOrResume();
}

string smc::cht_det::MinioDataSaver::loadFile(const string &url) {
    auto result = client_->download(bucketName, url);
    return result;
}

void smc::cht_det::MinioDataSaver::saveFile(const string &content, const string &url) {
    client_->upload(content, bucketName, url);
}

void smc::cht_det::MinioDataSaver::saveImage(const cv::Mat &image, const string &url,
                                             const vector<int> &vecCompression_params) {
    client_->upload(image, bucketName, url, vecCompression_params);
}

