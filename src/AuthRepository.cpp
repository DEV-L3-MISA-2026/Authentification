#include <AuthRepository.h>
#include <LocalException.h>
#include <utility.h> 
#include <iostream>
#include <stdexcept>

std::shared_ptr<AuthRepository> AuthRepository::instance = nullptr;

AuthRepository::AuthRepository(const std::string& connectionString) 
    : conn(connectionString) 
{
    std::cout << "Connexion réussie à la base de données 'auth'." << std::endl;
}

void AuthRepository::init(const std::string& connectionString) 
{
    if (!instance) {
        instance.reset(new AuthRepository(connectionString));
    }
}

std::shared_ptr<AuthRepository> AuthRepository::getInstance() 
{
    if (!instance) {
        throw std::runtime_error("AuthRepository n'est pas initialisé. Appelez init() en premier.");
    }
    return instance;
}

int AuthRepository::getIdByUsername(const std::string& username) 
{
    pqxx::nontransaction tx(this->conn);
    std::string sql = "SELECT id FROM users WHERE username=$1";
    pqxx::result rows = tx.exec_params(sql, username);

    if (rows.empty()) {
        return -1;
    } else {
        return rows[0][0].as<int>();
    }
}

// Utilisation de la struct ici
void AuthRepository::insertEmbeddings(const AuthData& data) 
{
    // On accède au nom d'utilisateur via data.username
    int id = this->getIdByUsername(data.username);
    if (id == -1) {
        throw LocalException("cannot find the username in the db !");
    }

    // Vérification des dimensions depuis la struct
    if (data.faceEmbedding.size() != 128) {
        throw LocalException("bad face embedding shape, we need a 128 vector!");
    }
    if (data.voiceEmbedding.size() != 192) {
        throw LocalException("bad voice embedding shape, we need a 192 vector!");
    }

    // Conversion des vecteurs
    std::string face_embedding_str = vector_to_pgstring(data.faceEmbedding);
    std::string voice_embedding_str = vector_to_pgstring(data.voiceEmbedding);
    
    pqxx::work tx(this->conn);
    std::string sql = "INSERT INTO auth_data (userid, face_embendding, voice_embendding) VALUES ($1, $2, $3)";
    
    try {
        tx.exec_params(sql, id, face_embedding_str, voice_embedding_str);
        tx.commit(); 
    } catch (const pqxx::sql_error& err) {
        std::string detail_erreur = "Erreur SQL : " + std::string(err.what()) + "\n" +
                                    "Requête exécutée : " + err.query();
        throw std::runtime_error(detail_erreur);
    } catch (const std::exception& err) {
        throw std::runtime_error("Erreur standard : " + std::string(err.what()));
    }
}