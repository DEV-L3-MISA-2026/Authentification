#include <Controllers/FaceController.h>
#include <string>
#include <utility.h>   // set_cors
#include <Services/AuthService.h>
FaceController::FaceController(std::shared_ptr<FaceService> faceService,  std::shared_ptr<AuthService> authService)
    : faceService_(std::move(faceService)), authService_(std::move(authService)) {}

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
         std::string token = json["tokens"].s();
        int userId = json["uid"].i();
        std::string image_b64 = json["img"].s();
 // 1. Decode & verify the JWT (throws on tampered/expired token).
        AuthSession session;
        try {
            session = authService_->decode(token);
        } catch (const std::exception& ex) {
            res.code = 401;
            set_cors(res);
            res.write("{\"success\":false,\"error\":\"Invalid token: " + std::string(ex.what()) + "\"}");
            res.end();
            return;
        }
 
        // 2. userId must belong to this session.
        if (!authService_->containsUser(session, userId)) {
            res.code = 403;
            set_cors(res);
            res.write("{\"success\":false,\"userId\":" + std::to_string(userId) +
                       ",\"error\":\"userId is not part of this session\"}");
            res.end();
            return;
        }
 
        // 3. Facial is the first factor in the chain -- no prerequisite to gate on.
        FaceVerifyResult result = faceService_->verify(userId, image_b64);
 
        res.set_header("Content-Type", "application/json");
        set_cors(res);
 
        if (!result.success) {
            // computeEmbedding/lookup failure (bad image, no face detected, not enrolled, ...)
            res.code = result.httpCode;
            res.write("{\"success\":false,\"userId\":" + std::to_string(userId) +
                       ",\"error\":\"" + result.error + "\"}");
            res.end();
            return;
        }
 
        if (!result.verified) {
            res.code = 200;
            char buf[512];
            std::cout << "score" << result.similarity << "\n";
            snprintf(buf, sizeof(buf),
                "{\"success\":false,\"userId\":%d,\"similarity\":%.4f,\"error\":\"Face does not match enrolled data\"}",
                userId, result.similarity);
            res.write(buf);
            res.end();
            return;
        }
 
        // 4. Mark facial done, re-sign the JWT, respond.
        authService_->markFactorDone(session, userId, AuthFactor::Facial);
        std::string newToken = authService_->encode(session);
 
        res.code = 200;
        char buf[1024];
        std::cout << "score" << result.similarity << "\n";

        
        snprintf(buf, sizeof(buf),
            "{\"token\":\"%s\",\"status\":\"success\",\"verified\":%s,\"userId\":%d,\"similarity\":%.4f}",
            newToken.c_str(), result.verified ? "true" : "false", userId, result.similarity);
        res.write(buf);
        res.end();
    } catch (const std::exception& e) {
        res.code = 500;
        set_cors(res);
        std::cout << "error " << std::string(e.what()) << "\n";
        res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
        res.end();
    }
}