#include <Controllers/VocController.h>

#include <iostream>

VocController::VocController(std::shared_ptr<VocService> vocService)
    : vocService_(std::move(vocService)) {}

crow::response VocController::verify(const crow::request& req, int userId) {
    std::cout << "receiving " << req.body.size() << " bytes!" << std::endl;

    VocVerifyResult result = vocService_->verify(userId, req.body.data(), req.body.size());

    crow::json::wvalue res;
    res["allowed"] = result.allowed;
    res["score"] = result.score;
    return crow::response(res);
}

crow::response VocController::set(const crow::request& req, int userId) {
    std::cout << "receiving " << req.body.size() << " bytes!" << std::endl;

    try {
        vocService_->setEmbedding(userId, req.body.data(), req.body.size());
    } catch (const std::exception& err) {
        return crow::response(500, "Sorry, some error occured in the server, please contact the devs");
    }

    return crow::response("Ok");
}