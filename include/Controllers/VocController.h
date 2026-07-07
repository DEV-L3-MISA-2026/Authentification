#pragma once

#include <memory>
#include <crow.h>

#include <Services/VocService.h>

// Couche HTTP : parse la requete, appelle VocService, ecrit la reponse.
// A brancher depuis vos fichiers de routes existants.
class VocController {
public:
    explicit VocController(std::shared_ptr<VocService> vocService);

    crow::response verify(const crow::request& req, int userId);
    crow::response set(const crow::request& req, int userId);

private:
    std::shared_ptr<VocService> vocService_;
};