#include <iostream>
#include <memory>

#include <opencv2/opencv.hpp>
#include <crow.h>

#include <Repository/AuthRepository.h>
#include <FaceRecognizer.h>
#include <SpeechProcessor.h>
#include <Vad.h>

#include <Services/FaceService.h>
#include <Services/VocService.h>
#include <Controllers/FaceController.h>
#include <Controllers/VocController.h>
#include <Routes/FaceRoutes.hpp>
#include <Routes/VocRoutes.hpp>
#include <utility.h>

int main() {
    // init all models and tools
    Vad::init("../model/silero_vad.onnx");

    AuthRepository::init("dbname=auth user=postgres password=mamanlah host=localhost port=5432");
    std::shared_ptr<AuthRepository> authRepository = AuthRepository::getInstance();

    SpeechProcessor::init("../model/speech_detection.onnx");
    std::shared_ptr<SpeechProcessor> speechProcessor = SpeechProcessor::getInstance();

    cv::Ptr<fr::FaceDetector> faceDetector = cv::makePtr<fr::FaceDetector>(
        "../model/face_detection_yunet_2023mar.onnx",
        cv::Size(320, 320),
        0.8,
        0.4,
        fr::backend_target_pairs.at(0).first,
        fr::backend_target_pairs.at(0).second);

    std::shared_ptr<fr::FaceRecognizer> faceRecognizer =
        std::make_shared<fr::FaceRecognizer>(faceDetector, "../model/face_recognition_sface_2021dec.onnx");

    // ---- construction des services
    auto faceService = std::make_shared<FaceService>(faceDetector, faceRecognizer, authRepository);
    auto vocService = std::make_shared<VocService>(speechProcessor, authRepository);

    // ---- construction des controllers
    auto faceController = std::make_shared<FaceController>(faceService);
    auto vocController = std::make_shared<VocController>(vocService);

    // ---- crow and routings
    crow::SimpleApp app;

    CROW_ROUTE(app, "/").methods("OPTIONS"_method)([]() {
        crow::response res;
        set_cors(res);
        res.code = 204;
        res.end();
        return res;
    });

    CROW_ROUTE(app, "/health")([]() {
        crow::response res(200, "OK");
        set_cors(res);
        return res;
    });

    registerFaceRoutes(app, faceController);
    registerVocRoutes(app, vocController);

    std::cout << "Starting REST API server on http://0.0.0.0:7007" << std::endl;
    std::cout << "Endpoints:" << std::endl;
    std::cout << "  POST /api/face/enroll      - Register face for a user" << std::endl;
    std::cout << "  POST /api/face/verify      - Verify face against registered user" << std::endl;
    std::cout << "  POST /api/voc/verify/<id>  - Verify voice against registered user" << std::endl;
    std::cout << "  POST /api/voc/set/<id>     - Register/update voice embedding for a user" << std::endl;

    app.port(7007).multithreaded().run();

    return 0;
}