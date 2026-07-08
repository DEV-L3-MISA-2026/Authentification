#include "../include/FaceDetector.h"
#include "../include/FaceRecognizer.h"
#include "../include/Repository/AuthRepository.h"
#include "../include/utility.h"
#include "../include/AuthData.h"
#include "../include/FacesModel.h"
#include <crow.h>
#include <crow/middlewares/cors.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

std::vector<uchar> base64_decode(const std::string& in) {
    std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::vector<uchar> out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;
    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return out;
}

int main() {
    // Chaîne de connexion : priorité à la variable d'environnement PG_CONN,
    // sinon valeur par défaut (postgres / s sur localhost:5432).
    const char* pg_conn_env = std::getenv("PG_CONN");
    std::string pg_conn = pg_conn_env
        ? std::string(pg_conn_env)
        : "dbname=biometrika user=postgres password=s host=localhost port=5432";
    AuthRepository::init(pg_conn);

    cv::Ptr<fr::FaceDetector> face_detector = cv::makePtr<fr::FaceDetector>(
        "../model/face_detection_yunet_2023mar.onnx",
        cv::Size(320, 320), 0.8, 0.4,
        fr::backend_target_pairs.at(0).first,
        fr::backend_target_pairs.at(0).second
    );
    fr::FaceRecognizer face_recognizer(face_detector, "../model/face_recognition_sface_2021dec.onnx");

    // On utilise le middleware CORSHandler natif de Crow au lieu de headers
    // manuels : Crow répond automatiquement aux requêtes OPTIONS (preflight)
    // AVANT même d'atteindre les routes définies plus bas, quelle que soit la
    // méthode enregistrée sur la route (c'est ce qui expliquait le 204 sans
    // header CORS malgré nos tentatives précédentes). Seul ce middleware
    // intercepte correctement le preflight et y ajoute les bons headers.
    crow::App<crow::CORSHandler> app;
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors
        .global()
        .origin("*")
        .headers("Content-Type")
        .methods("GET"_method, "POST"_method, "OPTIONS"_method);

    // Health check
    CROW_ROUTE(app, "/health")([]() {
        return "OK";
    });

    // Enroll endpoint - registers a face for a user
    CROW_ROUTE(app, "/enroll").methods("POST"_method)([&](const crow::request& req, crow::response& res) {
        try {
            auto json = crow::json::load(req.body);
            if (!json) {
                res.code = 400;
                res.write("{\"error\":\"Invalid JSON\"}");
                res.write("{\"status\":\"success\"}");
                res.end();
                return;
            }

            std::string username = json["username"].s();
            std::string image_b64 = json["image"].s();

            if (username.empty() || image_b64.empty()) {
                res.code = 400;
                res.write("{\"error\":\"username and image are required\"}");
                res.end();
                return;
            }

            size_t comma_pos = image_b64.find(',');
            if (comma_pos != std::string::npos) image_b64 = image_b64.substr(comma_pos + 1);

            std::vector<uchar> img_data = base64_decode(image_b64);
            if (img_data.empty()) {
                res.code = 400;
                res.write("{\"error\":\"Invalid base64 image\"}");
                res.end();
                return;
            }

            cv::Mat img = cv::imdecode(img_data, cv::IMREAD_COLOR);
            if (img.empty()) {
                res.code = 400;
                res.write("{\"error\":\"Failed to decode image\"}");
                res.end();
                return;
            }

            cv::Mat faces = face_detector->GetMatFace(img);
            if (faces.empty() || faces.rows == 0) {
                res.code = 400;
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
                res.write("{\"error\":\"User not found\"}");
                res.end();
                return;
            }

            repo->setFacialEmbeddingsById(user_id, embedding);

            res.code = 200;
            res.set_header("Content-Type", "application/json");
            std::string body = "{\"status\":\"success\",\"message\":\"Face enrolled successfully\",\"username\":\"" + username + "\"}";
            res.write(body);
            res.end();
        } catch (const std::exception& e) {
            res.code = 500;
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
                res.write("{\"error\":\"Invalid JSON\"}");
                res.end();
                return;
            }

            std::string username = json["username"].s();
            std::string image_b64 = json["image"].s();

            if (username.empty() || image_b64.empty()) {
                res.code = 400;
                res.write("{\"error\":\"username and image are required\"}");
                res.end();
                return;
            }

            size_t comma_pos = image_b64.find(',');
            if (comma_pos != std::string::npos) image_b64 = image_b64.substr(comma_pos + 1);

            std::vector<uchar> img_data = base64_decode(image_b64);
            if (img_data.empty()) {
                res.code = 400;
                res.write("{\"error\":\"Invalid base64 image\"}");
                res.end();
                return;
            }

            cv::Mat img = cv::imdecode(img_data, cv::IMREAD_COLOR);
            if (img.empty()) {
                res.code = 400;
                res.write("{\"error\":\"Failed to decode image\"}");
                res.end();
                return;
            }

            cv::Mat faces = face_detector->GetMatFace(img);
            if (faces.empty() || faces.rows == 0) {
                res.code = 400;
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
                res.write("{\"error\":\"User not found\"}");
                res.end();
                return;
            }

            std::vector<float> stored_embedding = repo->getFacialEmbenddingById(user_id);
            if (stored_embedding.empty()) {
                res.code = 400;
                res.write("{\"error\":\"No face data enrolled for user\"}");
                res.end();
                return;
            }

            double similarity = cosineSimilarity(live_embedding, stored_embedding);
            bool verified = similarity > 0.5;

            res.code = 200;
            res.set_header("Content-Type", "application/json");
            char buf[512];
            snprintf(buf, sizeof(buf),
                "{\"status\":\"success\",\"verified\":%s,\"similarity\":%.4f,\"username\":\"%s\"}",
                verified ? "true" : "false", similarity, username.c_str());
            res.write(buf);
            res.end();
        } catch (const std::exception& e) {
            res.code = 500;
            std::string body = "{\"error\":\"" + std::string(e.what()) + "\"}";
            res.write(body);
            res.end();
        }
    });

    // WASM-compatible endpoints for Blazor frontend (CORS handled in responses)
    // POST /wasm/face/enroll - Register face for a user
    // Le preflight OPTIONS est intercepté et géré automatiquement par le
    // middleware CORSHandler configuré plus haut ; pas besoin de le gérer ici.
    CROW_ROUTE(app, "/wasm/face/enroll").methods("POST"_method)([&](const crow::request& req, crow::response& res) {
        try {
            auto json = crow::json::load(req.body);
            if (!json) {
                res.code = 400;
                res.write("{\"Success\":false,\"Message\":\"Invalid JSON\"}");
                res.end();
                return;
            }

            // Tolérant à la casse : on accepte "UserId" ou "userId"
            int user_id = 0;
            if (json.has("UserId")) user_id = json["UserId"].i();
            else if (json.has("userId")) user_id = json["userId"].i();

            std::string image_b64;
            if (json.has("ImageData")) image_b64 = json["ImageData"].s();
            else if (json.has("imageData")) image_b64 = json["imageData"].s();
            else if (json.has("image")) image_b64 = json["image"].s();

            if (user_id <= 0 || image_b64.empty()) {
                res.code = 400;
                res.write("{\"Success\":false,\"Message\":\"UserId and ImageData are required\"}");
                res.end();
                return;
            }

            size_t comma_pos = image_b64.find(',');
            if (comma_pos != std::string::npos) image_b64 = image_b64.substr(comma_pos + 1);

            std::vector<uchar> img_data = base64_decode(image_b64);
            if (img_data.empty()) {
                res.code = 400;
                res.write("{\"Success\":false,\"Message\":\"Invalid base64 image\"}");
                res.end();
                return;
            }

            cv::Mat img = cv::imdecode(img_data, cv::IMREAD_COLOR);
            if (img.empty()) {
                res.code = 400;
                res.write("{\"Success\":false,\"Message\":\"Failed to decode image\"}");
                res.end();
                return;
            }

            cv::Mat faces = face_detector->GetMatFace(img);
            if (faces.empty() || faces.rows == 0) {
                res.code = 400;
                res.write("{\"Success\":false,\"Message\":\"No face detected in image\"}");
                res.end();
                return;
            }

            cv::Mat characteristic = face_recognizer.GetCharacteristic(img);
            std::shared_ptr<FacesModel> facemodel = FacesModel::GetInstance();
            std::vector<float> embedding = facemodel->formatEmbending(characteristic);

            auto repo = AuthRepository::getInstance();
            
            // Generate template ID (using user_id as template_id for simplicity)
            int template_id = user_id;

            repo->setFacialEmbeddingsById(user_id, embedding);

            res.code = 200;
            res.set_header("Content-Type", "application/json");
            std::string body = "{\"Success\":true,\"Message\":\"Template facial enregistré pour l'utilisateur " + std::to_string(user_id) + "\",\"TemplateId\":" + std::to_string(template_id) + "}";
            res.write(body);
            res.end();
        } catch (const std::exception& e) {
            res.code = 500;
            std::string body = "{\"Success\":false,\"Message\":\"" + std::string(e.what()) + "\"}";
            res.write(body);
            res.end();
        }
    });

    // POST /wasm/face/verify - Verify face against registered user
    // Le preflight OPTIONS est intercepté et géré automatiquement par le
    // middleware CORSHandler configuré plus haut ; pas besoin de le gérer ici.
    CROW_ROUTE(app, "/wasm/face/verify").methods("POST"_method)([&](const crow::request& req, crow::response& res) {
        try {
            auto json = crow::json::load(req.body);
            if (!json) {
                res.code = 400;
                res.write("{\"Success\":false,\"Message\":\"Invalid JSON\"}");
                res.end();
                return;
            }

            // Tolérant à la casse : on accepte "TemplateId" ou "templateId"
            int template_id = 0;
            if (json.has("TemplateId")) template_id = json["TemplateId"].i();
            else if (json.has("templateId")) template_id = json["templateId"].i();

            std::string image_b64;
            if (json.has("ImageData")) image_b64 = json["ImageData"].s();
            else if (json.has("imageData")) image_b64 = json["imageData"].s();
            else if (json.has("image")) image_b64 = json["image"].s();

            if (template_id <= 0 || image_b64.empty()) {
                res.code = 400;
                res.write("{\"Success\":false,\"Message\":\"TemplateId and ImageData are required\"}");
                res.end();
                return;
            }

            size_t comma_pos = image_b64.find(',');
            if (comma_pos != std::string::npos) image_b64 = image_b64.substr(comma_pos + 1);

            std::vector<uchar> img_data = base64_decode(image_b64);
            if (img_data.empty()) {
                res.code = 400;
                res.write("{\"Success\":false,\"Message\":\"Invalid base64 image\"}");
                res.end();
                return;
            }

            cv::Mat img = cv::imdecode(img_data, cv::IMREAD_COLOR);
            if (img.empty()) {
                res.code = 400;
                res.write("{\"Success\":false,\"Message\":\"Failed to decode image\"}");
                res.end();
                return;
            }

            cv::Mat faces = face_detector->GetMatFace(img);
            if (faces.empty() || faces.rows == 0) {
                res.code = 400;
                res.write("{\"Success\":false,\"Message\":\"No face detected in image\"}");
                res.end();
                return;
            }

            cv::Mat characteristic = face_recognizer.GetCharacteristic(img);
            std::shared_ptr<FacesModel> facemodel = FacesModel::GetInstance();
            std::vector<float> live_embedding = facemodel->formatEmbending(characteristic);

            auto repo = AuthRepository::getInstance();
            // Using template_id directly as user_id for facial data lookup
            std::vector<float> stored_embedding = repo->getFacialEmbenddingById(template_id);
            if (stored_embedding.empty()) {
                res.code = 400;
                res.write("{\"Success\":false,\"Message\":\"No face data enrolled for user\"}");
                res.end();
                return;
            }

            double similarity = cosineSimilarity(live_embedding, stored_embedding);
            bool verified = similarity > 0.5;

            res.code = 200;
            res.set_header("Content-Type", "application/json");
            char buf[512];
            int score = static_cast<int>(similarity * 100);
            snprintf(buf, sizeof(buf),
                "{\"Success\":%s,\"Message\":\"Vérification faciale %s\",\"Score\":%d,\"TemplateId\":%d}",
                verified ? "true" : "false",
                verified ? "réussie" : "échouée",
                score, template_id);
            res.write(buf);
            res.end();
        } catch (const std::exception& e) {
            res.code = 500;
            std::string body = "{\"Success\":false,\"Message\":\"" + std::string(e.what()) + "\"}";
            res.write(body);
            res.end();
        }
    });

    std::cout << "Starting REST API server on http://0.0.0.0:5238" << std::endl;
    std::cout << "Endpoints:" << std::endl;
    std::cout << "  POST /wasm/face/enroll - Register face for a user" << std::endl;
    std::cout << "  POST /wasm/face/verify - Verify face against registered user" << std::endl;

    app.port(5238).multithreaded().run();

    return 0;
}