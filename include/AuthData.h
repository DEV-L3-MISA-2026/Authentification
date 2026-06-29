#pragma once
#include <vector>
#include <string>

struct AuthData {
    int userid;
    std::string username;
    std::vector<float> faceEmbedding;
    std::vector<float> voiceEmbedding;
};