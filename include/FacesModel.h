#ifndef __FACES_MODEL
#define __FACES_MODEL
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>
class FacesModel {
    public:
        static std::shared_ptr<FacesModel> GetInstance();
        int getIdByUsername(const std::string& username);
        std::vector<float> formatEmbending(cv::Mat matEmbending);
        void insertFaceEmbending(const std::string& username, std::vector<float> embending);
    private:
        pqxx::connection conn;
        static std::shared_ptr<FacesModel> instance;
        FacesModel();
};
#endif