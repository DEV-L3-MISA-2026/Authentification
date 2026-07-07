#include <Controllers/FaceController.h>

#include <utility.h>   // set_cors

FaceController::FaceController(std::shared_ptr<FaceService> faceService)
    : faceService_(std::move(faceService)) {}

void FaceController::enroll(const crow::request& req, crow::response& res) {
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

        FaceEnrollResult result = faceService_->enroll(username, image_b64);

        res.code = result.httpCode;
        set_cors(res);
        res.set_header("Content-Type", "application/json");

        if (!result.success) {
            res.write("{\"error\":\"" + result.error + "\"}");
        } else {
            res.write("{\"status\":\"success\",\"message\":\"Face enrolled successfully\",\"username\":\""
                       + result.username + "\"}");
        }
        res.end();
    } catch (const std::exception& e) {
        res.code = 500;
        set_cors(res);
        res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
        res.end();
    }
}

void FaceController::verify(const crow::request& req, crow::response& res) {
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

        FaceVerifyResult result = faceService_->verify(username, image_b64);

        res.code = result.httpCode;
        set_cors(res);
        res.set_header("Content-Type", "application/json");

        if (!result.success) {
            res.write("{\"error\":\"" + result.error + "\"}");
        } else {
            char buf[512];
            snprintf(buf, sizeof(buf),
                "{\"status\":\"success\",\"verified\":%s,\"similarity\":%.4f,\"username\":\"%s\"}",
                result.verified ? "true" : "false", result.similarity, result.username.c_str());
            res.write(buf);
        }
        res.end();
    } catch (const std::exception& e) {
        res.code = 500;
        set_cors(res);
        res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
        res.end();
    }
}