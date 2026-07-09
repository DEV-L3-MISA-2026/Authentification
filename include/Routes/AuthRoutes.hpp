 #pragma once
#include <crow.h>

#include <Controllers/AuthController.h>

template <typename AppType>
inline void registerAuthRoutes(AppType& app, std::shared_ptr<AuthController> authController)
{
    CROW_ROUTE(app, "/api/auth")
    .methods("POST"_method)
    ([authController](const crow::request&req, crow::response& res)
    {
        authController->initAuthSession(req, res);
    });
}