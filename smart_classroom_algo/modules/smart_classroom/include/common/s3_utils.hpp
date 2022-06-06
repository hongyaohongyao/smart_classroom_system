
#ifndef SMART_CLASSROOM_CPP_S3_UTILS_HPP
#define SMART_CLASSROOM_CPP_S3_UTILS_HPP

#include <string>
#include <opencv2/opencv.hpp>
#include <aws/s3/S3Client.h>
#include <aws/core/Aws.h>
#include <common/ilogger.hpp>

namespace smc::s3_utils {
    void initAwsAPI(bool shutdown = false);

    class MinioClient {
        std::shared_ptr<Aws::S3::S3Client> client_;
    public:
        MinioClient(const std::string &endpoint, const std::string &accessKey, const std::string &secretKey);

        ~MinioClient() {
            client_.reset();
            INFO("close MinioClient");
        }

        bool
        upload(const cv::Mat &img, const std::string &bucketName,
               const std::string &objectKey, const std::vector<int> &vecCompression_params = {});

        bool upload(const std::string &contents, const std::string &bucketName,
                    const std::string &objectKey);

        std::string download(const std::string &bucketName, const std::string &objectKey);

        cv::Mat downloadImg(const std::string &bucketName, const std::string &objectKey);
    };
}
#endif //SMART_CLASSROOM_CPP_S3_UTILS_HPP
