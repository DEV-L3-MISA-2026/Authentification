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
    // initializing all processor
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

    // defining the api routes of the vocal signature
    crow::SimpleApp app;

    // POST /api/voc/verify/:id
    // this route is used to verify the vocal signature of the voc of user : id
    // in the db, and the voc given in the body
    CROW_ROUTE(app, "/api/voc/verify/<int>").methods(crow::HTTPMethod::POST)
    ([&sp, &repo] (const crow::request& req, int userid) {
        std::cout << "receiving " << req.body.size() << "bytes!" << std::endl;
        
        // saving into .ogg file for test purposes
        std::ofstream file("voice.ogg", std::ios::binary);
        file.write(req.body.data(), req.body.size());
        
        // extracting the embending directly from the data
        std::vector<float> embendding = sp->getEmbendingFromData(req.body.data(), req.body.size());
        
        // extracting from db
        std::vector<float> userEmbending = repo->getVocalEmbenddingById(userid);
        float cosin_score =  cosineSimilarity(embendding, userEmbending);

        crow::json::wvalue res;
        res["allowed"] = "false";
        res["score"] = cosin_score;
        return crow::response(res);
    });


    // POST /api/voc/set/:id
    // this route is used to set the vocal embending of user : id 
    // to the embending of the voc posted in the body
    CROW_ROUTE(app, "/api/voc/set/<int>").methods(crow::HTTPMethod::POST)
    ([&sp, &repo] (const crow::request& req, int userid) {
        std::cout << "receiving " << req.body.size() << "bytes!" << std::endl;
        
        // extracting the embending directly from the data
        std::vector<float> embendding = sp->getEmbendingFromData(req.body.data(), req.body.size());
        
        try 
        {
            repo->setVocalEmbeddingById(userid, embendding);
        } catch(std::exception err) 
        {
            return crow::response(500, "Sorry, some error occured in the server, please contact the devs");
        }
        return crow::response("Ok");
    });
    app.port(5000).multithreaded().run();
    return 0;
}