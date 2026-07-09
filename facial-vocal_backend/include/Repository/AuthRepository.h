#pragma once

#include <memory>
#include <string>
#include <vector>
#include <pqxx/pqxx>
#include <AuthData.h>
#include <vector>

class AuthRepository {
public:
    static void init(const std::string& connectionString);

    // Récupération de l'instance
    static std::shared_ptr<AuthRepository> getInstance();

    // Suppression de la copie
    AuthRepository(const AuthRepository&) = delete;
    AuthRepository& operator=(const AuthRepository&) = delete;

    // check if id is present in auth_data table
    bool hasInstance(int id);
    int getIdByUsername(const std::string& username);
    void insertEmbeddings(const AuthData& data);
    void setFacialEmbeddingsById(int id, std::vector<float> embendding);
    void setVocalEmbeddingById(int id, std::vector<float> embedding);
    std::vector<float> getVocalEmbenddingById(int id);
    std::vector<float> getFacialEmbenddingById(int id);
private:
    // Constructeur privé
    AuthRepository(const std::string& connectionString);

    pqxx::connection conn;
    static std::shared_ptr<AuthRepository> instance;
};