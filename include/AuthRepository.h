#pragma once

#include <memory>
#include <string>
#include <vector>
#include <pqxx/pqxx>
#include <AuthData.h>


class AuthRepository {
public:
    static void init(const std::string& connectionString);

    // Récupération de l'instance
    static std::shared_ptr<AuthRepository> getInstance();

    // Suppression de la copie
    AuthRepository(const AuthRepository&) = delete;
    AuthRepository& operator=(const AuthRepository&) = delete;

    int getIdByUsername(const std::string& username);
    void insertEmbeddings(const AuthData& data);

private:
    // Constructeur privé
    AuthRepository(const std::string& connectionString);

    pqxx::connection conn;
    static std::shared_ptr<AuthRepository> instance;
};