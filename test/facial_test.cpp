#include "FaceDetector.h"
#include "FaceRecognizer.h"
#include <fstream>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <pqxx/pqxx>
#include "FacesModel.h"
#include <memory>

pqxx::connection init_connexion() {
    // static data, need to be loaded from a json file
    pqxx::connection conn("dbname=auth user=kukuna password=mamanlah");

    if (!conn.is_open())
        throw std::runtime_error("couldn't open the connection ");

    return conn;
}

void save_into_db(cv::Mat embeding, const std::string& username, pqxx::connection conn) {   
    
}

int main() {
    cv::Ptr<fr::FaceDetector> face_detector = cv::makePtr<fr::FaceDetector>("../model/face_detection_yunet_2023mar.onnx", cv::Size(320, 320), 0.8, 0.4, fr::backend_target_pairs.at(0).first, fr::backend_target_pairs.at(0).second);
    fr::FaceRecognizer face_recognizer(face_detector, "../model/face_recognition_sface_2021dec.onnx");

    // consider it as a sample from the db
    cv::Mat img = cv::imread("../me.jpeg");
    cv::Mat test_faces =face_detector->GetMatFace(img);
    cv::Mat characteristic = face_recognizer.GetCharacteristic(img);
    // std::cout << characteristic.size();
    std::shared_ptr<FacesModel> facemodel = FacesModel::GetInstance();
    std::vector<float> embending = facemodel->formatEmbending(characteristic);
    
    facemodel->insertFaceEmbending("Mikajy", embending);
    // getting stream from cam
    /*
        cv::VideoCapture cap;
        int device = 0;         // 0 = default cam
        int api = cv::CAP_ANY;  // choose dynamically
        cap.open(device, api);

        if (!cap.isOpened()) {
            std::cerr << " ERROR, unable to open stream " << device << std::endl;
            return -1;
        }
        cv::Mat frame, current_car, face;
        cv::Rect rect_face;
        int x, y, w, h;

        for(;;) {
            cap.read(frame);
            if(frame.empty())
            {
                std::cerr << "ERROR, blanck frame grabbed\n";
                break; 
            }

            // process the recognition
            cv::Mat face =face_detector->GetMatFace(frame);
            current_car = face_recognizer.GetCharacteristic(frame);
            if(!face.empty())
            {
                x = face.at<float>(0, 0);
                y = face.at<float>(0, 1);
                w = face.at<float>(0, 2);
                h = face.at<float>(0, 3);
                rect_face = cv::Rect(x, y, w, h);
                if(face_recognizer.Compare(characteristic, current_car))
                    // draw green circle
                    rectangle(frame, rect_face, cv::Scalar(0, 255, 0), 2);
                else
                    rectangle(frame, rect_face, cv::Scalar(0, 0, 255), 2);
            }
            
            cv::imshow("Live", frame);
            if (cv::waitKey(5) >= 0) break;
        }
    */
    return 0;
}