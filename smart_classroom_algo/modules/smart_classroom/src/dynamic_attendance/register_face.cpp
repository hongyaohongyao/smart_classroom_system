#include <dynamic_attendance/dynamic_attendance.hpp>
#include <common/curl_utils.hpp>
#include <common/models_utils.hpp>
#include <common/json_utils.hpp>

using namespace std;

cJSON *smc::dyn_att::feature2json(const cv::Mat_<float> &feature) {
    cJSON *array = cJSON_AS4CPP_CreateArray();
    for (float item:feature) {
        cJSON_AS4CPP_AddItemToArray(array, cJSON_AS4CPP_CreateNumber(item));
    }
    return array;
}

static cv::Mat_<float> json2feature(cJSON *array) {
    if (not cJSON_AS4CPP_IsArray(array)) {
        return {};
    }
    int size = cJSON_AS4CPP_GetArraySize(array);
    if (size <= 0) {
        return {};
    }
    cv::Mat_<float> feature(1, size);
    for (int i = 0; i < size; i++) {
        *feature[i] = (float) cJSON_AS4CPP_GetNumberValue(cJSON_AS4CPP_GetArrayItem(array, i));
    }
    return feature;
}


string smc::dyn_att::registerFace(const string &imgDir) {
    INFO("downloading img %s", imgDir.c_str());
    cv::Mat img = curl_utils::httpDownloadImg(imgDir.c_str());
    if (img.empty()) {
        INFOW("empty image find");
        return {};
    }
    auto detector = models_utils::getScrfd();
    auto arcface = models_utils::getArcface();

    auto faces = detector->commit(img).get();
    if (faces.empty()) {
        INFOW("no face detected");
        return {};
    }
    int max_face_index =
            (int) (std::max_element(faces.begin(), faces.end(),
                                    [](face_detector::Box &face1, face_detector::Box &face2) {
                                        return face1.area() < face2.area();
                                    }) - faces.begin());
    auto face = faces[max_face_index];
    auto cropped = scrfd::crop_face_and_landmark(img, face);
    auto feature = arcface->commit(cropped).get();
    cJSON *feature_obj = feature2json(feature);
    string &&result = json_utils::json2uf_str(feature_obj);
    cJSON_AS4CPP_Delete(feature_obj);

    return result;
}