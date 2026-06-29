#ifndef __SPEAKER_ENCODER
#define __SPEAKER_ENCODER
#include <string>
#include <memory>
#include <vector>
#include <onnxruntime/onnxruntime_cxx_api.h>
class SpeechProcessor
{
    public:
        static void init(const std::string& modelPath);
        static std::shared_ptr<SpeechProcessor> getInstance();
        std::vector<float> getEmbending(const std::string& path);
    private:
        SpeechProcessor(const std::string& modelPath);
        static std::shared_ptr<SpeechProcessor> instance;
        Ort::Env env;
        Ort::Session session;
};

#endif