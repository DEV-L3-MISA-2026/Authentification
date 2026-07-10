#include <Services/FaceService.h>

#include <utility.h>   
#include <FacesModel.h>

FaceService::FaceService(cv::Ptr<fr::FaceDetector> faceDetector,
                          std::shared_ptr<fr::FaceRecognizer> faceRecognizer,
                          std::shared_ptr<AuthRepository> authRepository,
                          double verificationThreshold)
    : faceDetector_(std::move(faceDetector)),
      faceRecognizer_(std::move(faceRecognizer)),
      authRepository_(std::move(authRepository)),
      verificationThreshold_(verificationThreshold) {}

bool FaceService::computeEmbedding(const std::string& imageBase64,
                                    std::vector<float>& outEmbedding,
                                    std::string& outError,
                                    int& outHttpCode) {
    std::string image = imageBase64;

    // on retire le prefixe "data:image/...;base64," si present
    size_t comma_pos = image.find(',');
    if (comma_pos != std::string::npos) {
        image = image.substr(comma_pos + 1);
    }

    std::vector<uchar> img_data = base64_decode(image);
    if (img_data.empty()) {
        outError = "Invalid base64 image";
        outHttpCode = 400;
        return false;
    }

    cv::Mat img = cv::imdecode(img_data, cv::IMREAD_COLOR);
    if (img.empty()) {
        outError = "Failed to decode image";
        outHttpCode = 400;
        return false;
    }

    cv::Mat faces = faceDetector_->GetMatFace(img);
    if (faces.empty() || faces.rows == 0) {
        outError = "No face detected in image";
        outHttpCode = 400;
        return false;
    }

    cv::Mat characteristic = faceRecognizer_->GetCharacteristic(img);
    std::shared_ptr<FacesModel> facemodel = FacesModel::GetInstance();
    outEmbedding = facemodel->formatEmbending(characteristic);
    return true;
}

FaceEnrollResult FaceService::enroll(const std::string& username, const std::string& imageBase64) {
    FaceEnrollResult result{};
    result.username = username;

    if (username.empty() || imageBase64.empty()) {
        result.success = false;
        result.httpCode = 400;
        result.error = "username and image are required";
        return result;
    }

    std::vector<float> embedding;
    std::string error;
    int httpCode = 200;
    if (!computeEmbedding(imageBase64, embedding, error, httpCode)) {
        result.success = false;
        result.httpCode = httpCode;
        result.error = error;
        return result;
    }

    int user_id = authRepository_->getIdByUsername(username);
    if (user_id == -1) {
        result.success = false;
        result.httpCode = 404;
        result.error = "User not found";
        return result;
    }

    authRepository_->setFacialEmbeddingsById(user_id, embedding);

    result.success = true;
    result.httpCode = 200;
    return result;
}

FaceVerifyResult FaceService::verify(int uid, const std::string& imageBase64) {
    FaceVerifyResult result{};
    result.id = uid;

    std::vector<float> live_embedding;
    std::string error;
    int httpCode = 200;
    if (!computeEmbedding(imageBase64, live_embedding, error, httpCode)) {
        result.success = false;
        result.httpCode = httpCode;
        result.error = error;
        return result;
    }

    int user_id = uid;
    if (user_id == -1) {
        result.success = false;
        result.httpCode = 404;
        result.error = "User not found";
        return result;
    }

    std::vector<float> stored_embedding = authRepository_->getFacialEmbenddingById(user_id);
    if (stored_embedding.empty()) {
        result.success = false;
        result.httpCode = 400;
        result.error = "No face data enrolled for user";
        return result;
    }

    double similarity = cosineSimilarity(live_embedding, stored_embedding);

    result.success = true;
    result.httpCode = 200;
    result.similarity = similarity;
    result.verified = similarity > verificationThreshold_;
    return result;
}