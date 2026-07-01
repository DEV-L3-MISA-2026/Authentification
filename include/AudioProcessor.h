#ifndef __AUDIO_PROCESSOR
#define __AUDIO_PROCESSOR
#include <vector>
#include <string>
#include <memory>

class AudioProcessor {
    public:
        std::vector<float> resample(const std::vector<float>& input, int inputRate, int outputRate);
        std::vector<float> stereoToMono(const std::vector<float>& input);
        std::vector<float> loadAudio(const std::string& path);
        std::vector<float> loadAudio(const void* data, size_t size);
        std::vector<float> loadAudioForCleaning(const std::string& path);
        std::vector<float> cleanAudioRNNoise(const std::vector<float>& input);

        static std::shared_ptr<AudioProcessor> getInstance();
    private:
        static std::shared_ptr<AudioProcessor> instance;
        AudioProcessor();
};
#endif