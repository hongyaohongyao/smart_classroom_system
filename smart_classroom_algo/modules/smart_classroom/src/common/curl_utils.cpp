#include "common/curl_utils.hpp"
#include <common/ilogger.hpp>

using namespace std;

static size_t writer(char *data, size_t size, size_t nmemb, string *writerData) {
    if (writerData == nullptr)
        return 0;
    size_t len = size * nmemb;
    writerData->append(data, len);
    return len;
}

void smc::curl_utils::postUpLoad(const cv::Mat &img, const char *url) {
    vector<uchar> vec_Img;

    vector<int> vecCompression_params;

    vecCompression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    vecCompression_params.push_back(90);
    imencode(".jpg", img, vec_Img, vecCompression_params);

    CURL *curl;
    CURLcode res;

    string buffer;

    struct curl_httppost *post = nullptr;
    struct curl_httppost *last = nullptr;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        struct curl_slist *chunk = nullptr;
        chunk = curl_slist_append(chunk, "Accept:");

        chunk = curl_slist_append(chunk, "Authorization: Token ");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);


        curl_formadd(&post, &last,
                     CURLFORM_COPYNAME, "photo",
                     CURLFORM_BUFFER, "photo.jpg",
                     CURLFORM_BUFFERPTR, vec_Img.data(),
                     CURLFORM_BUFFERLENGTH, vec_Img.size(),
                     CURLFORM_CONTENTTYPE, "image/jpg",
                     CURLFORM_END);

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

        //            curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        //
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        curl_easy_cleanup(curl);
    }
}

struct TmpStream {
    void *ptr;
    size_t last_num;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, TmpStream *stream) {
    if (stream->last_num <= 0)
        return 0;
    size_t retcode = size * nmemb;
    retcode = MIN(retcode, stream->last_num);
    memcpy(ptr, stream->ptr, retcode);
    stream->last_num -= retcode;
//        curl_off_t nread = (curl_off_t) retcode;
//        fprintf(stderr, " We read %" CURL_FORMAT_CURL_OFF_T
//                        " bytes from file\n", nread);
    return retcode;
}

void smc::curl_utils::ftpUpload(const cv::Mat &img, const char *url, const char *user_pwd, int quality) {
    vector<int> vecCompression_params;
    vecCompression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    vecCompression_params.push_back(quality);
    ftpUpload(img, url, vecCompression_params, user_pwd);
}

void
smc::curl_utils::ftpUpload(const cv::Mat &img, const char *url, const vector<int> &vecCompression_params,
                           const char *user_pwd) {
    vector<uchar> vec_Img;
    imencode(".jpg", img, vec_Img, vecCompression_params);

    //        struct curl_slist *headerlist = nullptr;
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_USERPWD, user_pwd);
        /* we want to use our own write function */
        //            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
        /* we want to use our own read function */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

        /**create missing dir**/
        curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

        /* enable uploading */
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        /* specify target */
        curl_easy_setopt(curl, CURLOPT_URL, url);

        //            /* pass in that last of FTP commands to run after the transfer */
        //            curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);

        /* now specify which file to upload */
        TmpStream stream = {vec_Img.data(), vec_Img.size()};
        curl_easy_setopt(curl, CURLOPT_READDATA, &stream);

        /* Set the size of the file to upload (optional).  If you give a *_LARGE
        option you MUST make sure that the type of the passed-in argument is a
         curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
          make sure that to pass in a type 'long' argument. */
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                         (curl_off_t) vec_Img.size());
        /* Now run off and do what you have been told! */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* clean up the FTP commands list */
        //            curl_slist_free_all(headerlist);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
}

void smc::curl_utils::ftpUpload(const string &contents, const char *url, const char *user_pwd) {
    //        struct curl_slist *headerlist = nullptr;
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_USERPWD, user_pwd);
        /* we want to use our own write function */
        //            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
        /* we want to use our own read function */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

        /**create missing dir**/
        curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

        /* enable uploading */
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        /* specify target */
        curl_easy_setopt(curl, CURLOPT_URL, url);

        //            /* pass in that last of FTP commands to run after the transfer */
        //            curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);

        /* now specify which file to upload */
        TmpStream stream = {(void *) contents.data(), contents.size()};
        curl_easy_setopt(curl, CURLOPT_READDATA, &stream);

        /* Set the size of the file to upload (optional).  If you give a *_LARGE
        option you MUST make sure that the type of the passed-in argument is a
         curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
          make sure that to pass in a type 'long' argument. */
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                         (curl_off_t) contents.size());
        /* Now run off and do what you have been told! */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* clean up the FTP commands list */
        //            curl_slist_free_all(headerlist);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
}

string smc::curl_utils::ftpDownload(const char *url, const char *user_pwd) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    string buffer;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_USERPWD, user_pwd);
        /* specify target */
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* clean up the FTP commands list */
        //            curl_slist_free_all(headerlist);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    return buffer;
}


cv::Mat smc::curl_utils::httpDownloadImg(const char *url) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    string buffer;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* clean up the FTP commands list */
        //            curl_slist_free_all(headerlist);

        /* always cleanup */
        curl_easy_cleanup(curl);
        vector<uchar> img_vec(buffer.begin(), buffer.end());
        auto &&result = cv::imdecode(img_vec, cv::IMREAD_COLOR);
        if (result.empty()) {
            INFOE("%s", buffer.c_str());
        }
        return result;
    }
    return {};
}