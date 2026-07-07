#pragma once

#include <memory>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include <FaceRecognizer.h>
#include <Repository/AuthRepository.h>

// Resultat d'un enrolement de visage
struct FaceEnrollResult {
    bool     success;
    int      httpCode;
    std::string error;      // vide si success == true
    std::string username;
};

// Resultat d'une verification de visage
struct FaceVerifyResult {
    bool     success;
    int      httpCode;
    std::string error;      // vide si success == true
    std::string username;
    bool     verified = false;
    double   similarity = 0.0;
};

// Contient toute la logique metier liee a la reconnaissance faciale.
// Ne connait rien du protocole HTTP : ne fait que retourner des structures de resultat.
class FaceService {
public:
    FaceService(cv::Ptr<fr::FaceDetector> faceDetector,
                std::shared_ptr<fr::FaceRecognizer> faceRecognizer,
                std::shared_ptr<AuthRepository> authRepository,
                double verificationThreshold = 0.5);

    FaceEnrollResult enroll(const std::string& username, const std::string& imageBase64);
    FaceVerifyResult verify(const std::string& username, const std::string& imageBase64);

private:
    cv::Ptr<fr::FaceDetector>            faceDetector_;
    std::shared_ptr<fr::FaceRecognizer>  faceRecognizer_;
    std::shared_ptr<AuthRepository>      authRepository_;
    double                               verificationThreshold_;

    // Decode le base64 -> image -> detection -> embedding
    // Retourne false + message d'erreur + code http si une etape echoue
    bool computeEmbedding(const std::string& imageBase64,
                           std::vector<float>& outEmbedding,
                           std::string& outError,
                           int& outHttpCode);
};