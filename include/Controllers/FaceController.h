#pragma once

#include <memory>
#include <crow.h>

#include <Services/FaceService.h>

// Couche HTTP : parse la requete, appelle FaceService, ecrit la reponse.
// A brancher depuis vos fichiers de routes existants.
class FaceController {
public:
    explicit FaceController(std::shared_ptr<FaceService> faceService);

    void enroll(const crow::request& req, crow::response& res);
    void verify(const crow::request& req, crow::response& res);

private:
    std::shared_ptr<FaceService> faceService_;
};