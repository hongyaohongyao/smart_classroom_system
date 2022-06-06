#include <cheating_detection/data_saver.hpp>

#define DESC_BLOCK_SIZE 50

using namespace std;

smc::cht_det::BaseDataSaver::BaseDataSaver(const string &baseUrl) : DataSaver(),
        async_utils::SingletonThread<Data>() {
    this->baseUrl = baseUrl;
    this->descUrl = this->baseUrl + "/" + DESC_FILE;
    /**图片压缩参数**/
    vecCompression_params.push_back(cv::IMWRITE_JPEG_QUALITY); /**保存jpg格式**/
    vecCompression_params.push_back(90); /**压缩质量90%**/
    /**初始化snapshot的id**/
    curIdx = 0;
    curJsonArray = nullptr;
    /**派生类的构造函数记得最后记得手动调用一下initOrResume()**/
}

void smc::cht_det::BaseDataSaver::initOrResume() {
    auto content = loadFile(descUrl);
    if (not content.empty()) { // if exist desc file,resume desc state
        /**解析json文件**/
        cJSON *jsonObj = cJSON_AS4CPP_Parse(content.c_str());
        cJSON *curIdxObj = cJSON_AS4CPP_GetObjectItemCaseSensitive(jsonObj, "curIdx");
        curIdx = static_cast<size_t>(cJSON_AS4CPP_GetNumberValue(curIdxObj));
        cJSON_AS4CPP_Delete(jsonObj);
    } else {
        saveDescFile();
    }

    if (curIdx % DESC_BLOCK_SIZE != 0) {
        char fileName[1024];
        snprintf(fileName, 1024, "%s/%ld.desc", baseUrl.c_str(), curIdx / DESC_BLOCK_SIZE);
        auto content_ = loadFile(fileName);
        //            cout << fileName << "  " << content_ << "dsadasdasdasdadsas======================" << endl;
        curJsonArray = cJSON_AS4CPP_Parse(content_.c_str());

    } else {
        curJsonArray = cJSON_AS4CPP_CreateArray();
    }
}

void smc::cht_det::BaseDataSaver::save(const Data &data) {
    async_utils::SingletonThread<Data>::commit(data);
}

struct SaveObj {
    int chtType;
    cv::Mat snapshot;

    explicit SaveObj(int chtType, cv::Mat snapshot) :
            chtType(chtType),
            snapshot(std::move(snapshot)) {
    }
};


void smc::cht_det::BaseDataSaver::process(const Data &data) {
    const auto &cheatingStates = data.cheatingStates.get();
    const auto &snapshots = data.snapshots.get();
    vector<SaveObj> saveObjs;
    for (int i = 0; i < cheatingStates.size(); ++i) {
        auto &chtStat = cheatingStates[i];
        bool cond = (chtStat.predState > 0) && (chtStat.predState == chtStat.cheatingState) && (chtStat.fitTimes == 6);
        if (cond) {
            const auto &snapshot = snapshots[i];
            saveObjs.emplace_back(chtStat.predState, snapshot);
        }
    }
    /**save json and snapshot**/
    if (not saveObjs.empty()) {// save result
        char fileName[1024];
        size_t objsNum = saveObjs.size();
        for (int i = 0; i < objsNum; ++i) {
            size_t idx = curIdx++; //get new id
            auto &saveObj = saveObjs[i];
            /**add info to json array**/
            cJSON *jsonObj = cJSON_AS4CPP_CreateObject();
            cJSON_AS4CPP_AddNumberToObject(jsonObj, "chtType", saveObj.chtType); // add chtType
            cJSON_AS4CPP_AddItemToArray(curJsonArray, jsonObj);
            /**save snapshot**/
            snprintf(fileName, 1024, "%s/%ld.jpg", baseUrl.c_str(), idx);
            saveImage(saveObj.snapshot, fileName, vecCompression_params);
            if (curIdx % DESC_BLOCK_SIZE == 0) {
                /**save desc json file**/
                snprintf(fileName, 1024, "%s/%ld.desc", baseUrl.c_str(), idx / DESC_BLOCK_SIZE);
                char *results = cJSON_AS4CPP_PrintUnformatted(curJsonArray);// get json string
                saveFile(results, fileName); // save desc
                cJSON_AS4CPP_free(results); // free print result from cJson
                /**create new desc json**/
                cJSON_AS4CPP_Delete(curJsonArray);
                curJsonArray = cJSON_AS4CPP_CreateArray();
            } else if (i == objsNum - 1) {
                /**save desc json file**/
                snprintf(fileName, 1024, "%s/%ld.desc", baseUrl.c_str(), idx / DESC_BLOCK_SIZE);
                char *results = cJSON_AS4CPP_PrintUnformatted(curJsonArray);// get json string
                saveFile(results, fileName); // save desc
                cJSON_AS4CPP_free(results); // free print result from cJson
            }
        }
        /**update desc file**/
        saveDescFile();
    }
}

void smc::cht_det::BaseDataSaver::saveDescFile() {
    cJSON *jsonObj = cJSON_AS4CPP_CreateObject();

    cJSON_AS4CPP_AddNumberToObject(jsonObj, "curIdx", static_cast<double>(curIdx));
    {
        char *results = cJSON_AS4CPP_PrintUnformatted(jsonObj);// get json string
        saveFile(results, descUrl); // save desc
        cJSON_AS4CPP_free(results); // free print result from cJson
    }
    cJSON_AS4CPP_Delete(jsonObj);
}

