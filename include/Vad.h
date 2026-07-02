#ifndef __VAD
#define __VAD
#include <memory>
#include <string>
#include <vector>

#include <onnxruntime/onnxruntime_cxx_api.h>

class Vad { // using the silerovad model, this class removes all silence in an audio and concat it to form a new vector audio
    public:
        static void init(const std::string& modelpath);
        static std::shared_ptr<Vad> getInstance();
        bool isSpeech(const std::vector<float>& chunksAudio);                // check with the model if this chunks is a speech
        std::vector<float> removeSilence(const std::vector<float>& audio);  // removes all silence from audio and reconcat it into a new one
                                                                            // audio provided into this function must be treated with AudioProcessor first 
                                                                                    // to match the format required by the model (16khz, mono). 
        float infer(const std::vector<float>& frame);
        void reset();                                
        
    private:
        Vad(const std::string& modelpath);
        static std::shared_ptr<Vad> instance;
        Ort::Env env;
        Ort::Session session;

        std::vector<float> state;
        std::vector<float> context;

        static constexpr int WINDOW_SIZE = 512;
        static constexpr int CONTEXT_SIZE = 64;

        float threshold = 0.5f;
};
#endif