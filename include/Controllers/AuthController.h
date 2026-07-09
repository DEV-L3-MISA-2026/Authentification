#pragma once
#include <memory>
#include <Services/AuthService.h>
#include <crow.h>
class AuthController {
    public:
        explicit AuthController(std::shared_ptr<AuthService> fauthService);
        void initAuthSession(const crow::request& req, crow::response& res);
        
    private:
        std::shared_ptr<AuthService> authService_;
};