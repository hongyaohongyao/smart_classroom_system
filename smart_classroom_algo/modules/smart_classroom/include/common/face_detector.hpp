#ifndef SMART_CLASSROOM_CPP_FACE_DETECTOR_HPP
#define SMART_CLASSROOM_CPP_FACE_DETECTOR_HPP

#include <opencv2/opencv.hpp>
#include <vector>

namespace smc {
    namespace face_detector {

        struct Box {
            float left, top, right, bottom, confidence;
            float landmark[10];

            cv::Rect cvbox() const { return cv::Rect(left, top, right - left, bottom - top); }

            float width() const { return std::max(0.0f, right - left); }

            float height() const { return std::max(0.0f, bottom - top); }

            float area() const { return width() * height(); }

            float get_left() { return left; }

            void set_left(float value) { left = value; }

            float get_top() { return top; }

            void set_top(float value) { top = value; }

            float get_right() { return right; }

            void set_right(float value) { right = value; }

            float get_bottom() { return bottom; }

            void set_bottom(float value) { bottom = value; }

            float get_confidence() { return confidence; }

            void set_confidence(float value) { confidence = value; }
        };

        typedef std::vector<Box> BoxArray;
    };
}
#endif // SMART_CLASSROOM_CPP_FACE_DETECTOR_HPP