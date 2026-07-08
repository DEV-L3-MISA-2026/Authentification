#ifndef __VOICE_ROUTE
#define __VOICE_ROUTE
#include <crow.h>
#include <Controllers/VocController.h>

template <typename AppType>
inline void registerVocRoutes(AppType& app, std::shared_ptr<VocController> vocController) {
    // verify the vocal signature of userid against the audio provided in the body
    CROW_ROUTE(app, "/api/voc/verify/<int>")
        .methods("POST"_method)
        ([vocController](const crow::request& req, int userId) {
            return vocController->verify(req, userId);
        });
 
    // set/update the vocal embedding of userid from the audio provided in the body
    CROW_ROUTE(app, "/api/voc/set/<int>")
        .methods("POST"_method)
        ([vocController](const crow::request& req, int userId) {
            return vocController->set(req, userId);
        });
}

#endif