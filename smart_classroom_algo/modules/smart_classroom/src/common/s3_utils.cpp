#include "common/s3_utils.hpp"

#include "aws\core\auth\AWSCredentialsProvider.h"

#include "aws\s3\model\PutObjectRequest.h"
#include "aws\s3\model\GetObjectRequest.h"
#include <common/ilogger.hpp>

#undef GetObject
#undef GetMessage


void smc::s3_utils::initAwsAPI(bool shutdown) {
    static bool isInited = false;
    static Aws::SDKOptions options;
    if (not isInited && not shutdown) {
        Aws::InitAPI(options);
        isInited = true;
    } else if (isInited && shutdown) {
        Aws::ShutdownAPI(options);
        isInited = false;
    }
}

smc::s3_utils::MinioClient::MinioClient(const std::string &endpoint, const std::string &accessKey,
                                        const std::string &secretKey) {
    initAwsAPI();
    Aws::Client::ClientConfiguration cfg;
    cfg.endpointOverride = endpoint;  // S3服务器地址和端口
    INFO("endpoint %s access_key %s secret_key %s", endpoint.c_str(), accessKey.c_str(), secretKey.c_str());
    cfg.scheme = Aws::Http::Scheme::HTTP;
    cfg.verifySSL = false;
    client_ = std::make_shared<Aws::S3::S3Client>(Aws::Auth::AWSCredentials(accessKey, secretKey), cfg,
                                                  Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
}

bool smc::s3_utils::MinioClient::upload(const cv::Mat &img,
                                        const std::string &bucketName,
                                        const std::string &objectKey, const std::vector<int> &vecCompression_params) {
    std::vector<uchar> vec_Img;
    imencode(".jpg", img, vec_Img, vecCompression_params);
    Aws::S3::Model::PutObjectRequest putObjectRequest;
    putObjectRequest.WithBucket(bucketName).WithKey(objectKey);
    auto img_stream = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream");
    img_stream->write((char *) vec_Img.data(), (long long) vec_Img.size());
    putObjectRequest.SetBody(img_stream);
    auto putObjectResult = client_->PutObject(putObjectRequest);
    if (not putObjectResult.IsSuccess()) {
        INFO(putObjectResult.GetError().GetMessage().c_str());
        return false;
    }
    return true;
}

bool smc::s3_utils::MinioClient::upload(const std::string &contents, const std::string &bucketName,
                                        const std::string &objectKey) {
    Aws::S3::Model::PutObjectRequest putObjectRequest;
    putObjectRequest.WithBucket(bucketName).WithKey(objectKey);
    auto contents_stream = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream", contents);
    putObjectRequest.SetBody(contents_stream);
    auto putObjectResult = client_->PutObject(putObjectRequest);
    if (not putObjectResult.IsSuccess()) {
        INFO(putObjectResult.GetError().GetMessage().c_str());
        return false;
    }
    return true;
}

std::string smc::s3_utils::MinioClient::download(const std::string &bucketName, const std::string &objectKey) {
    INFO("bucket: %s dir: %s", bucketName.c_str(), objectKey.c_str());
    Aws::S3::Model::GetObjectRequest object_request;
    object_request.WithBucket(bucketName).WithKey(objectKey);
    auto get_object_outcome = client_->GetObject(object_request);
    if (get_object_outcome.IsSuccess()) {
        std::ostringstream tmp;
        tmp << get_object_outcome.GetResult().GetBody().rdbuf();
        return tmp.str();
    } else {
        INFO(get_object_outcome.GetError().GetMessage().c_str());
        return "";
    }
}

cv::Mat smc::s3_utils::MinioClient::downloadImg(const std::string &bucketName, const std::string &objectKey) {
    INFO("downloading img bucket: %s dir: %s", bucketName.c_str(), objectKey.c_str());
    Aws::S3::Model::GetObjectRequest object_request;
    object_request.WithBucket(bucketName).WithKey(objectKey);
    auto get_object_outcome = client_->GetObject(object_request);
    if (get_object_outcome.IsSuccess()) {
        auto &body = get_object_outcome.GetResult().GetBody();
        std::vector<uchar> vec_img;
        vec_img.assign(Aws::IStreamBufIterator(body), Aws::IStreamBufIterator());
        cv::Mat result = cv::imdecode(vec_img, cv::IMREAD_COLOR);
        return result;
    } else {
        INFO(get_object_outcome.GetError().GetMessage().c_str());
        return {};
    }
}