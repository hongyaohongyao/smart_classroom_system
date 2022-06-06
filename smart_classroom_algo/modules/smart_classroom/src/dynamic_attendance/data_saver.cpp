#include <dynamic_attendance/data_saver.hpp>
#include <fstream>

using namespace std;
using namespace smc;

smc::dyn_att::LocalDataSaver::LocalDataSaver(const std::string &baseUrl, Info &info) : BaseDataSaver(baseUrl, info) {
    if (iLogger::mkdirs(this->baseUrl)) {
        INFO("create directory success");
    }
    recoverDescFile();
}

string smc::dyn_att::LocalDataSaver::loadFile(const string &url) {
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

void smc::dyn_att::LocalDataSaver::saveFile(const string &content, const string &url) {
    ofstream outfile(url);
    outfile << content;
    outfile.close(); // close file
}

void smc::dyn_att::LocalDataSaver::saveImage(const cv::Mat &image, const string &url,
                                             const vector<int> &vecCompression_params) {
    cv::imwrite(url, image, vecCompression_params);
}


smc::dyn_att::MinioDataSaver::MinioDataSaver(dyn_att::Info &info, const string &endPoint,
                                             const string &accessKey,
                                             const string &secretKey,
                                             const string &bucketName,
                                             const string &baseUrl) : BaseDataSaver(baseUrl, info),
                                                                      endPoint(endPoint),
                                                                      accessKey(accessKey),
                                                                      secretKey(secretKey),
                                                                      bucketName(bucketName) {
    client_ = make_shared<s3_utils::MinioClient>(endPoint, accessKey, secretKey);
    recoverDescFile();
}

string smc::dyn_att::MinioDataSaver::loadFile(const string &url) {
    auto result = client_->download(bucketName, url);
    return result;
}

void smc::dyn_att::MinioDataSaver::saveFile(const string &content, const string &url) {
    client_->upload(content, bucketName, url);
}

void smc::dyn_att::MinioDataSaver::saveImage(const cv::Mat &image, const string &url,
                                             const vector<int> &vecCompression_params) {
    client_->upload(image, bucketName, url, vecCompression_params);
}

