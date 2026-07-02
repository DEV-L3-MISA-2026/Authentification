#include <Vad.h> 
#include <iostream>

std::shared_ptr<Vad> Vad::instance = nullptr;
void Vad::init(const std::string& modelpath) {
    if (instance != nullptr)
        throw std::runtime_error("error while loading vad : trying to init vad two times!");
    instance.reset(new Vad(modelpath));

}

Vad::Vad(const std::string& modelPath) :
env(ORT_LOGGING_LEVEL_WARNING, "speaker"),
session(env, modelPath.c_str(), Ort::SessionOptions{})
{

}

std::shared_ptr<Vad> Vad::getInstance() {
    if (instance == nullptr)
        throw std::runtime_error("trying to access unitialized Vad!");
    return instance;
}

bool Vad::isSpeech(const std::vector<float>& frame)
{
    if (frame.size() != 512)
        throw std::runtime_error("Silero expects 512 samples.");

    Ort::MemoryInfo memoryInfo =
        Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator,
            OrtMemTypeDefault);

    //------------------------------------
    // input
    //------------------------------------

    std::array<int64_t, 2> inputShape{1, 512};

    Ort::Value audioTensor =
        Ort::Value::CreateTensor<float>(
            memoryInfo,
            const_cast<float*>(frame.data()),
            frame.size(),
            inputShape.data(),
            inputShape.size());

    //------------------------------------
    // sample rate
    //------------------------------------

    int64_t sr = 16000;

    std::array<int64_t,1> srShape{1};

    Ort::Value srTensor =
        Ort::Value::CreateTensor<int64_t>(
            memoryInfo,
            &sr,
            1,
            srShape.data(),
            srShape.size());

    //------------------------------------
    // state
    //------------------------------------

    std::array<int64_t,3> stateShape{2,1,128};

    Ort::Value stateTensor =
        Ort::Value::CreateTensor<float>(
            memoryInfo,
            state.data(),
            state.size(),
            stateShape.data(),
            stateShape.size());

    //------------------------------------
    // inference
    //------------------------------------

    std::array<const char*,3> inputNames =
    {
        "input",
        "state",
        "sr"
    };

    std::array<const char*,2> outputNames =
    {
        "output",
        "stateN"
    };

    auto outputs =
        session.Run(
            Ort::RunOptions{nullptr},
            inputNames.data(),
            std::array<Ort::Value,3>{
                std::move(audioTensor),
                std::move(stateTensor),
                std::move(srTensor)
            }.data(),
            3,
            outputNames.data(),
            2);

    float probability =
        outputs[0].GetTensorMutableData<float>()[0];

    float* newState =
        outputs[1].GetTensorMutableData<float>();

    std::copy(
        newState,
        newState + state.size(),
        state.begin());
    std::cout <<  (probability >= 0.5) << " with probability : " << probability << "\n";
    return probability >= 0.5;
}

std::vector<float> Vad::removeSilence(const std::vector<float>& audio) // removes all silence from audio and reconcat it into a new one
{
    state.assign(2 * 1 * 128, 0.f);

    std::vector<float> cleaned;

    constexpr size_t frameSize = 512;

    for (size_t i = 0; i < audio.size(); i += frameSize)
    {
        std::vector<float> frame(frameSize, 0.f);

        size_t remaining =
            std::min(frameSize, audio.size() - i);

        std::copy(
            audio.begin() + i,
            audio.begin() + i + remaining,
            frame.begin());

        if (isSpeech(frame))
        {
            cleaned.insert(
                cleaned.end(),
                frame.begin(),
                frame.begin() + remaining);
        }
    }
    return cleaned;
}  
