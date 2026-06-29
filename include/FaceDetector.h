#ifndef __FACE_DETECTOR
#define __FACE_DETECTOR
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/face.hpp>

namespace fr {
    const std::vector<std::pair<int, int>> backend_target_pairs = {   // aliases for backend_id and target_id
        {cv::dnn::DNN_BACKEND_OPENCV, cv::dnn::DNN_TARGET_CPU},
        {cv::dnn::DNN_BACKEND_TIMVX,  cv::dnn::DNN_TARGET_NPU},
        {cv::dnn::DNN_BACKEND_CANN,   cv::dnn::DNN_TARGET_NPU}
    };

    class FaceDetector {    
        public:
            FaceDetector(
                std::string path_model,        // path to the yunet model
                const cv::Size& size,                  // size of the window, or the img to process, can be changed
                float conf_threshold,            // confidence threshold, minimum trust level to say its a face ( < 1)
                float nms_threshold,             // non maximum supression, for superposing rectangles
                int backend_id,                  // backend to use for the DNN
                int target_id                    // hardware to use
            );
            cv::Mat GetFace(cv::Mat img);        // return the higher probability face, cropped
            cv::Mat GetNFaces(cv::Mat img, int max_faces);
            cv::Mat GetMatFace(cv::Mat img);
        private:
            cv::Ptr<cv::FaceDetectorYN> face_detector;  // to put the yunet model
    };
}
#endif