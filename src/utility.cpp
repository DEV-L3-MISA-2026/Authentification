#include <utility.h>
#include <sstream>
#include <cmath>

std::vector<float> pgstring_to_vector(const std::string& str) {
    std::vector<float> vec;
    // On retire les crochets '[' et ']'
    std::string stripped = str.substr(1, str.size() - 2);
    std::stringstream ss(stripped);
    std::string value;
    
    while (std::getline(ss, value, ',')) {
        vec.push_back(std::stof(value));
    }
    return vec;
}

std::string vector_to_pgstring(const std::vector<float>& vec) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        ss << vec[i];
        if (i < vec.size() - 1) ss << ",";
    }
    ss << "]";
    return ss.str();
}

double cosineSimilarity(
    const std::vector<float>& a,
    const std::vector<float>& b)
{
    double dot = 0;
    double na = 0;
    double nb = 0;

    for(size_t i = 0; i < a.size(); i++)
    {
        dot += a[i] * b[i];
        na += a[i] * a[i];
        nb += b[i] * b[i];
    }

    return dot /
           (sqrt(na) * sqrt(nb));
}

double euclideanDistance(
    const std::vector<float>& a,
    const std::vector<float>& b)
{
    double sum = 0.0;

    for(size_t i = 0; i < a.size(); i++)
    {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }

    return std::sqrt(sum);
}