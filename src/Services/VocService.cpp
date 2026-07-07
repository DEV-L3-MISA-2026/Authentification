#include <Services/VocService.h>

#include <utility.h>   // cosineSimilarity

VocService::VocService(std::shared_ptr<SpeechProcessor> speechProcessor,
                        std::shared_ptr<AuthRepository> authRepository,
                        float verificationThreshold)
    : speechProcessor_(std::move(speechProcessor)),
      authRepository_(std::move(authRepository)),
      verificationThreshold_(verificationThreshold) {}

VocVerifyResult VocService::verify(int userId, const char* audioData, size_t audioSize) {
    std::vector<float> embedding = speechProcessor_->getEmbendingFromData(audioData, audioSize);
    // NB: nom conserve tel quel, typo presente dans AuthRepository::getVocalEmbenddingById
    std::vector<float> userEmbedding = authRepository_->getVocalEmbenddingById(userId);

    float score = cosineSimilarity(embedding, userEmbedding);

    VocVerifyResult result{};
    result.score = score;
    result.allowed = score > verificationThreshold_;
    return result;
}

void VocService::setEmbedding(int userId, const char* audioData, size_t audioSize) {
    std::vector<float> embedding = speechProcessor_->getEmbendingFromData(audioData, audioSize);
    authRepository_->setVocalEmbeddingById(userId, embedding);
}