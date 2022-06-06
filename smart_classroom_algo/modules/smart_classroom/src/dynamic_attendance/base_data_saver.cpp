#include <dynamic_attendance/data_saver.hpp>

using namespace std;

smc::dyn_att::BaseDataSaver::BaseDataSaver(const std::string &baseUrl, dyn_att::Info &info) : info(info) {
    this->baseUrl = baseUrl;
    this->descUrl = this->baseUrl + "/" + DESC_FILE;
    /**图片压缩参数**/
    vecCompression_params.push_back(cv::IMWRITE_JPEG_QUALITY); /**保存jpg格式**/
    vecCompression_params.push_back(90); /**压缩质量90%**/
    /**初始化snapshot的id**/
}


void smc::dyn_att::BaseDataSaver::recoverDescFile() {
    //不需要恢复，下面的代码非当前格式，仅供参考
    //    string content = loadFile(descUrl);
//    if (content.empty())
//        return;
//    cJSON *j_obj = cJSON_AS4CPP_Parse(content.c_str());
//    if (j_obj == nullptr)
//        return;
//    cJSON *j_matched = cJSON_AS4CPP_GetObjectItem(j_obj, "matched");
//    if (j_matched == nullptr)
//        return;
//    int size = (int) info.memInfos.size();
//    string idx;
//    cJSON *tmp_obj;
//    {
//        lock_guard<mutex> lk(info.sync.mutex);
//        for (int i = 0; i < size; ++i) {
//            idx = to_string(i);
//            cJSON *matchInfo = cJSON_AS4CPP_GetObjectItem(j_matched, idx.c_str());
//            if (matchInfo == nullptr)
//                continue;
//            long long int timestamp;
//            float confidence = 0;
//            tmp_obj = cJSON_AS4CPP_GetObjectItem(matchInfo, "timestamp");
//            if (tmp_obj != nullptr) {
//                timestamp = (long long int) cJSON_AS4CPP_GetNumberValue(tmp_obj);
//            } else {
//                timestamp = iLogger::timestamp_now();
//            }
//            tmp_obj = cJSON_AS4CPP_GetObjectItem(matchInfo, "confidence");
//            if (tmp_obj != nullptr) {
//                confidence = (float) cJSON_AS4CPP_GetNumberValue(tmp_obj);
//            }
//            if (confidence < info.unknownThres) {
//                continue;
//            }
//            info.sync.matched[i] = {timestamp, confidence};
//        }
//    }
//    cJSON_AS4CPP_Delete(j_obj);
}

void smc::dyn_att::BaseDataSaver::saveDescFile() {
    cJSON *j_obj = cJSON_AS4CPP_CreateObject();
    {
        lock_guard<mutex> lk(info.sync.mutex);
        auto &matched = info.sync.matched;
        string idx;
        cJSON *tmp_obj;
        for (auto &itr : matched) {
            tmp_obj = cJSON_AS4CPP_CreateObject();
            cJSON_AS4CPP_AddNumberToObject(tmp_obj, "timestamp", (double) itr.second.timestamp);
            cJSON_AS4CPP_AddNumberToObject(tmp_obj, "confidence", itr.second.confidence);
            cJSON_AS4CPP_AddNumberToObject(tmp_obj, "idx", (double) itr.first);
            cJSON_AS4CPP_AddItemToObject(j_obj, itr.second.memId.c_str(), tmp_obj);
        }
    }
    cJSON *j_save = cJSON_AS4CPP_CreateObject();
    cJSON_AS4CPP_AddNumberToObject(j_save, "timestamp", (double) iLogger::timestamp_now());
    cJSON_AS4CPP_AddItemToObject(j_save, "attendance", j_obj);

    char *str = cJSON_AS4CPP_PrintUnformatted(j_save);
    saveFile(str, descUrl);

    cJSON_AS4CPP_free(str);
    cJSON_AS4CPP_Delete(j_save);
}

void smc::dyn_att::BaseDataSaver::process(const Data &data) {
    auto &scores = data.scores.get();
    auto &snapshots = data.snapshots.get();
    int size = (int) scores.size();
    vector<tuple<int, int>> saveSnapshotIdx;
    {
        lock_guard<mutex> lk(info.sync.mutex);
        auto &matched = info.sync.matched;
        for (int i = 0; i < size; i++) {
            auto &faceScore = scores[i];
            if (faceScore.confidence < info.unknownThres) {
                continue;
            }
            auto itr = matched.find(faceScore.idx);
            if (itr == matched.end()) {
                auto &&memId = info.memInfos[faceScore.idx].memId;
                matched[faceScore.idx] = {iLogger::timestamp_now(), faceScore.confidence, memId};
                saveSnapshotIdx.emplace_back(i, faceScore.idx);
            } else {
                auto &matchInfo = itr->second;
                if (matchInfo.confidence < faceScore.confidence) {
                    matchInfo.confidence = faceScore.confidence;
                    matchInfo.timestamp = iLogger::timestamp_now();
                    saveSnapshotIdx.emplace_back(i, faceScore.idx);
                }
            }
        }
    }


    if (!saveSnapshotIdx.empty()) {
        for (auto &i:saveSnapshotIdx) {
            auto &&img = snapshots[get<0>(i)];
            saveImage(img, iLogger::format("%s/%d.jpg", baseUrl.c_str(), get<1>(i)), vecCompression_params);
        }
        saveDescFile();
    }

}

void smc::dyn_att::BaseDataSaver::save(const Data &data) {
    async_utils::SingletonThread<Data>::commit(data);
}