#include <Repository/AuthRepository.h>
#include <iostream>
#include <utility>
#include <FaceRecognizer.h>
#include <SpeechProcessor.h>
#include <opencv2/opencv.hpp>
#include <AuthData.h>
#include <utility.h>
#include <crow.h>
#include <fstream>

int main() {
    AuthRepository::init("dbname=auth user=postgres password=mamanlah host=localhost port=5432");
    auto repo = AuthRepository::getInstance();
    SpeechProcessor::init("../model/speech_detection.onnx");
    auto sp = SpeechProcessor::getInstance();
    
    cv::Ptr<fr::FaceDetector> face_detector = cv::makePtr<fr::FaceDetector>
            ("../model/face_detection_yunet_2023mar.onnx", cv::Size(320, 320),
            0.8, 
            0.4,
            fr::backend_target_pairs.at(0).first, 
            fr::backend_target_pairs.at(0).second);
            
    fr::FaceRecognizer face_recognizer(face_detector, "../model/face_recognition_sface_2021dec.onnx");

    crow::SimpleApp app;
    CROW_ROUTE(app, "/")
    ([] () {
        return "Hello world !";
    });

    CROW_ROUTE(app, "/api/voc/test").methods(crow::HTTPMethod::POST)
    ([&sp, &repo] (const crow::request& req) {
        std::cout << "receiving " << req.body.size() << "bytes!" << std::endl;

        // saving into .ogg file
        std::ofstream file("voice.ogg", std::ios::binary);
       file.write(req.body.data(), req.body.size());
        std::vector<float> embendding = sp->getEmbendingFromData(req.body.data(), req.body.size());
        std::vector<float> myEmbendding = repo->getVocalEmbenddingById(2);

        std::cout << "similarity : " << cosineSimilarity(embendding, myEmbendding);
        return crow::response("Ok");
    });
    app.port(5000).multithreaded().run();
    return 0;
}

    // cv::Mat img = cv::imread("../static_data/me.jpeg");
    // cv::Mat face_embending = face_recognizer.GetCharacteristic(img);
    // userData.username = "Mikajy";    
    // userData.faceEmbedding = formatEmbending(face_embending);
    // userData.voiceEmbedding = sp->getEmbending("../static_data/voice1.wav");

    // std::cout << "face: " << userData.faceEmbedding.size() << " voice: " << userData.voiceEmbedding.size() << "\n";
    // repo->setFacialEmbeddingsById(2, userData.faceEmbedding);
    // repo->setVocalEmbeddingById(2, userData.voiceEmbedding);
