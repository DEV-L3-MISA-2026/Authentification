#include <sndfile.h>
#include <onnxruntime/onnxruntime_cxx_api.h>
#include <samplerate.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <SpeechProcessor.h>

using namespace std;





// class SpeakerEncoder
// {
// public:

//     SpeakerEncoder(const string& modelPath)
//         :
//         env(ORT_LOGGING_LEVEL_WARNING, "speaker"),
//         session(env, modelPath.c_str(), Ort::SessionOptions{})
//     {
//     }

//     vector<float> encode(
//         const vector<float>& audio
//     )
//     {
//         vector<int64_t> shape =
//         {
//             1,
//             (int64_t)audio.size()
//         };

//         Ort::MemoryInfo mem =
//             Ort::MemoryInfo::CreateCpu(
//                 OrtArenaAllocator,
//                 OrtMemTypeDefault
//             );

//         Ort::Value input =
//             Ort::Value::CreateTensor<float>(
//                 mem,
//                 const_cast<float*>(audio.data()),
//                 audio.size(),
//                 shape.data(),
//                 shape.size()
//             );

//         const char* inputs[] =
//         {
//             "waveform"
//         };

//         const char* outputs[] =
//         {
//             "embedding"
//         };

//         auto out =
//             session.Run(
//                 Ort::RunOptions{nullptr},
//                 inputs,
//                 &input,
//                 1,
//                 outputs,
//                 1
//             );

//         float* ptr =
//             out[0].GetTensorMutableData<float>();

//         auto shapeOut =
//             out[0]
//             .GetTensorTypeAndShapeInfo()
//             .GetShape();

//         size_t n = 1;

//         for(auto s : shapeOut)
//             n *= s;

//         return vector<float>(
//             ptr,
//             ptr + n
//         );
//     }

// private:

//     Ort::Env env;

//     Ort::Session session;
// };

int main()
{
    // SpeakerEncoder encoder(
    //     "../model/speech_detection.onnx"
    // );

    // auto emb1 =
    //     encoder.encode(
    //         loadAudio("../static_data/voice1.wav")
    //     );

    // auto emb2 =
    //     encoder.encode(
    //         loadAudio("../static_data/bonjour.wav")
    //     );
    
    SpeechProcessor::init("../model/speech_detection.onnx");
    shared_ptr<SpeechProcessor> sp = SpeechProcessor::getInstance();
    
    auto emb1 =
    sp->getEmbending("../static_data/voice1.wav");

    auto emb2 =
    sp->getEmbending("../static_data/voice2.wav");

    
    cout << emb1.size() << endl;
    cout
        << cosineSimilarity(
            emb1,
            emb2
        )
        << endl;
}