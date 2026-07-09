#ifndef __UTILITY
#define __UTILITY
#include <vector>
#include <string>

#include <opencv2/opencv.hpp>
std::string vector_to_pgstring(const std::vector<float>& vec);
std::vector<float> pgstring_to_vector(const std::string& str);
double cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b);
double euclideanDistance(const std::vector<float>& a, const std::vector<float>& b);
std::vector<float> formatEmbending(cv::Mat matEmbending);
#endif