#include <Vad.h> 
#include <iostream>

std::shared_ptr<Vad> Vad::instance = nullptr;
void Vad::init(const std::string& modelpath) {
    if (instance != nullptr)
        throw std::runtime_error("error while loading vad : trying to init vad two times!");
    instance.reset(new Vad(modelpath));

}

void Vad::reset()
{
    state.assign(2 * 1 * 128, 0.f);
    context.assign(CONTEXT_SIZE, 0.f);
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
    return infer(frame) >= threshold;
}

float Vad::infer(const std::vector<float>& frame)
{
    if(frame.size() != WINDOW_SIZE)
        throw std::runtime_error("Expected 512 samples");

    //------------------------------------------
    // construit context + frame
    //------------------------------------------

    std::vector<float> input;
    input.reserve(CONTEXT_SIZE + WINDOW_SIZE);

    input.insert(
        input.end(),
        context.begin(),
        context.end());

    input.insert(
        input.end(),
        frame.begin(),
        frame.end());

    //------------------------------------------
    // prépare le prochain contexte
    //------------------------------------------

    context.assign(
        frame.end() - CONTEXT_SIZE,
        frame.end());

    //------------------------------------------
    // tensors
    //------------------------------------------

    Ort::MemoryInfo memory =
        Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator,
            OrtMemTypeDefault);

    std::array<int64_t,2> inputShape{
        1,
        static_cast<int64_t>(input.size())   // =576
    };

    Ort::Value inputTensor =
        Ort::Value::CreateTensor<float>(
            memory,
            input.data(),
            input.size(),
            inputShape.data(),
            inputShape.size());

    int64_t sr = 16000;

    Ort::Value srTensor =
        Ort::Value::CreateTensor<int64_t>(
            memory,
            &sr,
            1,
            nullptr,
            0);

    std::array<int64_t,3> stateShape{
        2,
        1,
        128
    };

    Ort::Value stateTensor =
        Ort::Value::CreateTensor<float>(
            memory,
            state.data(),
            state.size(),
            stateShape.data(),
            stateShape.size());

    const char* inputNames[]={
        "input",
        "state",
        "sr"
    };

    const char* outputNames[]={
        "output",
        "stateN"
    };

    Ort::Value inputs[]={
        std::move(inputTensor),
        std::move(stateTensor),
        std::move(srTensor)
    };

    auto outputs=
        session.Run(
            Ort::RunOptions{nullptr},
            inputNames,
            inputs,
            3,
            outputNames,
            2);

    float probability=
        outputs[0].GetTensorMutableData<float>()[0];

    float* newState=
        outputs[1].GetTensorMutableData<float>();

    std::copy(
        newState,
        newState + state.size(),
        state.begin());

    return probability;
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
