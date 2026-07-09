#ifndef __FACE_RECOGNIZER__
#define __FACE_RECOGNIZER__
#include <opencv2/opencv.hpp>
#include <string>

#include <FaceDetector.h>
namespace fr {

    class FaceDetector;

    class FaceRecognizer {
        public:
            FaceRecognizer(cv::Ptr<FaceDetector> face_detector, std::string model_path, int backend_id = 0, int target_id = 0);
            cv::Mat GetCharacteristic(cv::Mat img);
            bool Compare(cv::Mat source, cv::Mat target);
        private:
            cv::Ptr<FaceDetector> face_detector;
            cv::Ptr<cv::FaceRecognizerSF> face_recognizer;
            cv::Mat faces_from_db;
    };
}
#endif