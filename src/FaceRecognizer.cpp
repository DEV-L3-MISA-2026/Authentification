#include <FaceRecognizer.h>

namespace fr {
        FaceRecognizer::FaceRecognizer(cv::Ptr<FaceDetector> face_detector, std::string model_path, int backend_id, int target_id ) {
            this->face_detector = face_detector;
            this->face_recognizer = cv::FaceRecognizerSF::create(model_path, "", backend_id, target_id);
        }

        cv::Mat FaceRecognizer::GetCharacteristic(cv::Mat img) {
            cv::Mat faces = this->face_detector->GetMatFace(img);
            
            if (faces.empty()) {
                std::cerr << "There is no face." << std::endl;
                return cv::Mat(); 
            }

            cv::Mat aligned, features;
            face_recognizer->alignCrop(
                img,
                faces.row(0),
                aligned
            );

            face_recognizer->feature(
                aligned, 
                features
            );
            return features;
        }

        bool FaceRecognizer::Compare(cv::Mat source, cv::Mat target) {
            float cosin_score = this->face_recognizer->match(source, target, cv::FaceRecognizerSF::FR_COSINE);
            
            return cosin_score > 0.363;
        }
}