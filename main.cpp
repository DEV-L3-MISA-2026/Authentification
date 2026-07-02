#include <rnnoise.h>
#include <AudioProcessor.h>
#include <SpeechProcessor.h>
#include <fstream>
#include <iostream>
#include <utility.h>
#include <Vad.h>
using namespace std;

int main(int argc, char** argv) {
    if (argc != 3)
    {
        cout << "usage: ./app path_audio1 path_audio2" << endl;
        exit(1);
    }
    auto audioprocessor = AudioProcessor::getInstance();
    SpeechProcessor::init("../model/speech_detection.onnx");
    auto sp = SpeechProcessor::getInstance();
    // vad 
    Vad::init("../model/silero_vad.onnx");
    vector<float> audio1 = sp->getEmbending(string(argv[1]));
    vector<float> audio2 = sp->getEmbending(string(argv[2]));

    cout << "similarity of " << argv[1] << " and " << argv[2] << cosineSimilarity(audio1, audio2) << "\n"; 

    return 0;
}