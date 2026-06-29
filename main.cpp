#include <AuthRepository.h>
#include <iostream>
#include <utility>

int main() {
    AuthRepository::init("dbname=auth user=postgres password=mamanlah host=localhost port=5432");
    auto repo = AuthRepository::getInstance();
    
    AuthData userData;
    userData.username = "john";
    userData.faceEmbedding = std::vector<float>(128, 0.5f);
    userData.voiceEmbedding = std::vector<float>(192, 0.1f);

    try {
        repo->insertEmbeddings(userData);
        std::cout << "Embeddings insérés avec succès !" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }

    return 0;
}