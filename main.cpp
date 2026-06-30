#include <AuthRepository.h>
#include <iostream>
#include <utility>
#include <FaceRecognizer.h>
#include <SpeechProcessor.h>
#include <opencv2/opencv.hpp>
#include <AuthData.h>
#include <utility.h>

int main() {
    AuthRepository::init("dbname=auth user=postgres password=mamanlah host=localhost port=5432");
    auto repo = AuthRepository::getInstance();
    SpeechProcessor::init("../model/speech_detection.onnx");
    auto sp = SpeechProcessor::getInstance();
    
    cv::Ptr<fr::FaceDetector> face_detector = cv::makePtr<fr::FaceDetector>("../model/face_detection_yunet_2023mar.onnx", cv::Size(320, 320), 0.8, 0.4, fr::backend_target_pairs.at(0).first, fr::backend_target_pairs.at(0).second);
    fr::FaceRecognizer face_recognizer(face_detector, "../model/face_recognition_sface_2021dec.onnx");
    AuthData userData;

    cv::Mat img = cv::imread("../me.jpeg");
    cv::Mat face_embending = face_recognizer.GetCharacteristic(img);
    userData.username = "Mikajy";    
    userData.faceEmbedding = formatEmbending(face_embending);
    userData.voiceEmbedding = sp->getEmbending("../static_data/voice1.wav");


    cout << "face: " << userData.faceEmbedding.size() << " voice: " << userData.voiceEmbedding.size() << "\n";
    // try {
    //     repo->insertEmbeddings(userData);
    //     std::cout << "Embeddings insérés avec succès !" << std::endl;
    // } catch (const std::exception& e) {
    //     std::cerr << "Erreur : " << e.what() << std::endl;
    // }

    return 0;
}