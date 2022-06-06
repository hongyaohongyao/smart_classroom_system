#ifndef SMART_CLASSROOM_CPP_DATA_SAVER_HPP
#define SMART_CLASSROOM_CPP_DATA_SAVER_HPP

#include <dynamic_attendance/dynamic_attendance.hpp>
#include <common/s3_utils.hpp>

namespace smc::dyn_att {

    class BaseDataSaver : public DataSaver, public async_utils::SingletonThread<Data> {

    public:
        explicit BaseDataSaver(const std::string &baseUrl, dyn_att::Info &info);

        void save(const Data &data) override;


    protected:
        std::string baseUrl{};
        std::string descUrl{};
        const char *DESC_FILE = ".desc";
        std::vector<int> vecCompression_params{}; /**图像压缩参数**/

        dyn_att::Info &info;

        void process(const Data &data) override;

        void recoverDescFile();

        void saveDescFile();

        virtual std::string loadFile(const std::string &url) = 0;

        virtual void saveFile(const std::string &content, const std::string &url) = 0;

        virtual void
        saveImage(const cv::Mat &image, const std::string &url, const std::vector<int> &vecCompression_params) = 0;

    };

    class LocalDataSaver : public BaseDataSaver {
    public:
        explicit LocalDataSaver(const std::string &baseUrl, dyn_att::Info &info);

    protected:
        std::string loadFile(const std::string &url) override;

        void saveFile(const std::string &content, const std::string &url) override;

        void
        saveImage(const cv::Mat &image, const std::string &url,
                  const std::vector<int> &vecCompression_params) override;

    };

    class MinioDataSaver :
            public BaseDataSaver {
    private:
        std::shared_ptr<s3_utils::MinioClient> client_;
        std::string endPoint;
        std::string accessKey;
        std::string secretKey;
        std::string bucketName;

    public:
        explicit MinioDataSaver(dyn_att::Info &info, const std::string &endPoint,
                                const std::string &accessKey,
                                const std::string &secretKey,
                                const std::string &bucketName,
                                const std::string &baseUrl);

        ~MinioDataSaver() {
            stop();//stop behind deconstruct client;
        }

    protected:
        std::string loadFile(const std::string &url) override;

        void saveFile(const std::string &content, const std::string &url) override;

        void
        saveImage(const cv::Mat &image, const std::string &url,
                  const std::vector<int> &vecCompression_params) override;
    };
}

#endif //SMART_CLASSROOM_CPP_DATA_SAVER_HPP
