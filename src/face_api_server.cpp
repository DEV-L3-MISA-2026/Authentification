#include "../include/FaceDetector.h"
#include "../include/FaceRecognizer.h"
#include "../include/Repository/AuthRepository.h"
#include "../include/utility.h"
#include "../include/AuthData.h"
#include "../include/FacesModel.h"
#include <crow.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <utility.h>


int main() {
    AuthRepository::init("dbname=auth user=postgres password=s host=localhost port=5432");

    cv::Ptr<fr::FaceDetector> face_detector = cv::makePtr<fr::FaceDetector>(
        "../model/face_detection_yunet_2023mar.onnx",
        cv::Size(320, 320), 0.8, 0.4,
        fr::backend_target_pairs.at(0).first,
        fr::backend_target_pairs.at(0).second
    );
    fr::FaceRecognizer face_recognizer(face_detector, "../model/face_recognition_sface_2021dec.onnx");

    crow::SimpleApp app;

    CROW_ROUTE(app, "/").methods("OPTIONS"_method)([]() {
        crow::response res;
        set_cors(res);
        res.code = 204;
        res.end();
        return res;
    });

    // Health check
    CROW_ROUTE(app, "/health")([]() {
        crow::response res(200, "OK");
        set_cors(res);
        return res;
    });

    // Enroll endpoint - registers a face for a user
    CROW_ROUTE(app, "/enroll").methods("POST"_method)([&](const crow::request& req, crow::response& res) {
        try {
            auto json = crow::json::load(req.body);
            if (!json) {
                res.code = 400;
                set_cors(res);
                res.write("{\"error\":\"Invalid JSON\"}");
                res.write("{\"status\":\"success\"}");
                res.end();
                return;
            }

            std::string username = json["username"].s();
            std::string image_b64 = json["image"].s();

            if (username.empty() || image_b64.empty()) {
                res.code = 400;
                set_cors(res);
                res.write("{\"error\":\"username and image are required\"}");
                res.end();
                return;
            }

            size_t comma_pos = image_b64.find(',');
            if (comma_pos != std::string::npos) image_b64 = image_b64.substr(comma_pos + 1);

            std::vector<uchar> img_data = base64_decode(image_b64);
            if (img_data.empty()) {
                res.code = 400;
                set_cors(res);
                res.write("{\"error\":\"Invalid base64 image\"}");
                res.end();
                return;
            }

            cv::Mat img = cv::imdecode(img_data, cv::IMREAD_COLOR);
            if (img.empty()) {
                res.code = 400;
                set_cors(res);
                res.write("{\"error\":\"Failed to decode image\"}");
                res.end();
                return;
            }

            cv::Mat faces = face_detector->GetMatFace(img);
            if (faces.empty() || faces.rows == 0) {
                res.code = 400;
                set_cors(res);
                res.write("{\"error\":\"No face detected in image\"}");
                res.end();
                return;
            }

            cv::Mat characteristic = face_recognizer.GetCharacteristic(img);
            std::shared_ptr<FacesModel> facemodel = FacesModel::GetInstance();
            std::vector<float> embedding = facemodel->formatEmbending(characteristic);

            auto repo = AuthRepository::getInstance();
            int user_id = repo->getIdByUsername(username);
            if (user_id == -1) {
                res.code = 404;
                set_cors(res);
                res.write("{\"error\":\"User not found\"}");
                res.end();
                return;
            }

            repo->setFacialEmbeddingsById(user_id, embedding);

            res.code = 200;
            set_cors(res);
            res.set_header("Content-Type", "application/json");
            std::string body = "{\"status\":\"success\",\"message\":\"Face enrolled successfully\",\"username\":\"" + username + "\"}";
            res.write(body);
            res.end();
        } catch (const std::exception& e) {
            res.code = 500;
            set_cors(res);
            std::string body = "{\"error\":\"" + std::string(e.what()) + "\"}";
            res.write(body);
            res.end();
        }
    });

    // Verify endpoint - verifies a face against a user
    CROW_ROUTE(app, "/verify").methods("POST"_method)([&](const crow::request& req, crow::response& res) {
        try {
            auto json = crow::json::load(req.body);
            if (!json) {
                res.code = 400;
                set_cors(res);
                res.write("{\"error\":\"Invalid JSON\"}");
                res.end();
                return;
            }

            std::string username = json["username"].s();
            std::string image_b64 = json["image"].s();

            if (username.empty() || image_b64.empty()) {
                res.code = 400;
                set_cors(res);
                res.write("{\"error\":\"username and image are required\"}");
                res.end();
                return;
            }

            size_t comma_pos = image_b64.find(',');
            if (comma_pos != std::string::npos) image_b64 = image_b64.substr(comma_pos + 1);

            std::vector<uchar> img_data = base64_decode(image_b64);
            if (img_data.empty()) {
                res.code = 400;
                set_cors(res);
                res.write("{\"error\":\"Invalid base64 image\"}");
                res.end();
                return;
            }

            cv::Mat img = cv::imdecode(img_data, cv::IMREAD_COLOR);
            if (img.empty()) {
                res.code = 400;
                set_cors(res);
                res.write("{\"error\":\"Failed to decode image\"}");
                res.end();
                return;
            }

            cv::Mat faces = face_detector->GetMatFace(img);
            if (faces.empty() || faces.rows == 0) {
                res.code = 400;
                set_cors(res);
                res.write("{\"error\":\"No face detected in image\"}");
                res.end();
                return;
            }

            cv::Mat characteristic = face_recognizer.GetCharacteristic(img);
            std::shared_ptr<FacesModel> facemodel = FacesModel::GetInstance();
            std::vector<float> live_embedding = facemodel->formatEmbending(characteristic);

            auto repo = AuthRepository::getInstance();
            int user_id = repo->getIdByUsername(username);
            if (user_id == -1) {
                res.code = 404;
                set_cors(res);
                res.write("{\"error\":\"User not found\"}");
                res.end();
                return;
            }

            std::vector<float> stored_embedding = repo->getFacialEmbenddingById(user_id);
            if (stored_embedding.empty()) {
                res.code = 400;
                set_cors(res);
                res.write("{\"error\":\"No face data enrolled for user\"}\!");
                res.end();
                return;
            }

            double similarity = cosineSimilarity(live_embedding, stored_embedding);
            bool verified = similarity > 0.5;

            res.code = 200;
            set_cors(res);
            res.set_header("Content-Type", "application/json");
            char buf[512];
            snprintf(buf, sizeof(buf),
                "{\"status\":\"success\",\"verified\":%s,\"similarity\":%.4f,\"username\":\"%s\"}",
                verified ? "true" : "false", similarity, username.c_str());
            res.write(buf);
            res.end();
        } catch (const std::exception& e) {
            res.code = 500;
            set_cors(res);
            std::string body = "{\"error\":\"" + std::string(e.what()) + "\"}";
            res.write(body);
            res.end();
        }
    });

    std::cout << "Starting REST API server on http://0.0.0.0:7007" << std::endl;
    std::cout << "Endpoints:" << std::endl;
    std::cout << "  POST /enroll - Register face for a user" << std::endl;
    std::cout << "  POST /verify - Verify face against registered user" << std::endl;

    app.port(7007).multithreaded().run();

    return 0;
}
