#include <FaceDetector.h>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

namespace fr {
    FaceDetector::FaceDetector(
        std::string path_model,
        const cv::Size& input_size,
        float conf_threshold,
        float nms_threshold,
        int backend_id,
        int target_id
    ) {
        this->face_detector = cv::FaceDetectorYN::create(
            path_model, "", cv::Size(320, 320),  // we can change it later
            conf_threshold,
            nms_threshold,
            1, // top K
            backend_id,
            target_id
        );
    }

    cv::Mat FaceDetector::GetFace(cv::Mat img) {
        this->face_detector->setInputSize(img.size());
        this->face_detector->setTopK(1);
        
        // detect the main face
        cv::Mat faces;
        this->face_detector->detect(img, faces);

        if (faces.empty() || faces.rows == 0) {
            std::cout << "No face detected !" << std::endl;
            return img; // Ou retourne une matrice vide
        }
        // // crop it (remove into another image)
        int x = faces.at<float>(0, 0),
            y = faces.at<float>(0, 1),
            w = faces.at<float>(0, 2),
            h = faces.at<float>(0, 3);

        cv::Mat cropped = img(cv::Rect(x, y, w, h)).clone();
        //std::cout << x << " " << y << " " << w << " " << h << " " << std::endl;
        return cropped;
    }

    cv::Mat FaceDetector::GetMatFace(cv::Mat img) {
        this->face_detector->setInputSize(img.size());
        cv::Mat faces;

        this->face_detector->detect(img, faces);
        if (faces.empty() || faces.rows == 0) {
            std::cout << "Aucun visage détecté sur l'image !" << std::endl;
            return cv::Mat(); // <--- Retourner une matrice vide
        }
        return faces;
    }
}