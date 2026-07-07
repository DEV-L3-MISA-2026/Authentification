#pragma once

#include <memory>
#include <string>
#include <vector>

#include <SpeechProcessor.h>
#include <Repository/AuthRepository.h>

// Resultat d'une verification vocale
struct VocVerifyResult {
    bool   allowed;
    float  score;
};

// Contient toute la logique metier liee a la signature vocale.
// Ne connait rien du protocole HTTP.
class VocService {
public:
    VocService(std::shared_ptr<SpeechProcessor> speechProcessor,
               std::shared_ptr<AuthRepository> authRepository,
               float verificationThreshold = 0.5f);

    // Compare l'audio recu (raw bytes) a l'embedding enregistre pour userid
    VocVerifyResult verify(int userId, const char* audioData, size_t audioSize);

    // Enregistre / met a jour l'embedding vocal de userid a partir de l'audio recu
    // Lance une exception en cas d'erreur (a catcher dans le controller)
    void setEmbedding(int userId, const char* audioData, size_t audioSize);

private:
    std::shared_ptr<SpeechProcessor> speechProcessor_;
    std::shared_ptr<AuthRepository>  authRepository_;
    float                            verificationThreshold_;
};