#include <SpeechProcessor.h>
#include <AudioProcessor.h>

std::shared_ptr<SpeechProcessor> SpeechProcessor::instance = nullptr;

void SpeechProcessor::init(const std::string& modelPath){

    if (instance != nullptr)
        throw std::runtime_error("Speech processor have already been initialized !");

    instance.reset(new SpeechProcessor(std::move(modelPath)));
}

SpeechProcessor::SpeechProcessor(const std::string& modelPath)
    :
    env(ORT_LOGGING_LEVEL_WARNING, "speaker"),
    session(env, modelPath.c_str(), Ort::SessionOptions{})
{

}

std::vector<float> SpeechProcessor::getEmbending(const std::string& audioPath) {
    
    std::shared_ptr<AudioProcessor> audioprocessor = AudioProcessor::getInstance();
    
    std::vector<float> audio = audioprocessor->loadAudio(audioPath);
    std::vector<int64_t> shape = {1, (int64_t)audio.size()};

    // mem config
    Ort::MemoryInfo mem =
        Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator,
            OrtMemTypeDefault
        );

    // creating the tensor from the vector
    Ort::Value input =
        Ort::Value::CreateTensor<float>(
            mem,
            const_cast<float*>(audio.data()),
            audio.size(),
            shape.data(),
            shape.size()
        );

    const char* inputs[] = {"waveform"};
    const char* outputs[] = {"embedding"};

    auto out =
        this->session.Run(
            Ort::RunOptions{nullptr},
            inputs,
            &input,
            1,
            outputs,
            1
        );

    float* ptr = out[0].GetTensorMutableData<float>();

    auto shapeOut = out[0].GetTensorTypeAndShapeInfo().GetShape();
    size_t n = 1;
    for(auto s : shapeOut)
        n *= s;
    return std::vector<float>(
        ptr,
        ptr + n
    );
}

std::shared_ptr<SpeechProcessor> SpeechProcessor::getInstance() {
    if (instance == nullptr)
        throw std::runtime_error("Speech Processor must be initialized first with the init method !");
    return instance;
}