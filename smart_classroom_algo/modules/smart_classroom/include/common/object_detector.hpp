#ifndef SMART_CLASSROOM_CPP_OBJECT_DETECTOR_HPP
#define SMART_CLASSROOM_CPP_OBJECT_DETECTOR_HPP

#include <vector>

namespace smc::object_detector {

    struct Box {
        float left, top, right, bottom, confidence;
        int class_label;

        Box() = default;

        Box(float left, float top, float right, float bottom, float confidence, int class_label)
                : left(left), top(top), right(right), bottom(bottom), confidence(confidence),
                  class_label(class_label) {}
    };

    typedef std::vector<Box> BoxArray;
}
#endif // SMART_CLASSROOM_CPP_OBJECT_DETECTOR_HPP