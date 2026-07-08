#ifndef __FACE_ROUTE
#define __FACE_ROUTE
#include <crow.h>
#include <Controllers/FaceController.h>

template <typename AppType>
inline void registerFaceRoutes(AppType& app, std::shared_ptr<FaceController> faceController) {
    // enroll faces embedding
    CROW_ROUTE(app, "/api/face/enroll")
        .methods("POST"_method)
        ([faceController](const crow::request& req, crow::response& res) {
            faceController->enroll(req, res);
        });
 
    // verify face using the username and the img base_64 provided
    CROW_ROUTE(app, "/api/face/verify")
        .methods("POST"_method)
        ([faceController](const crow::request& req, crow::response& res) {
            faceController->verify(req, res);
        });
}
 

#endif