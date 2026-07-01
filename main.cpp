#include <rnnoise.h>
#include <AudioProcessor.h>
#include <SpeechProcessor.h>
#include <fstream>
#include <iostream>
#include <utility.h>

using namespace std;

int main() {
    auto audioprocessor = AudioProcessor::getInstance();
    SpeechProcessor::init("../model/speech_detection.onnx");

    auto sp = SpeechProcessor::getInstance();
    
    // testing the result
    vector<float> audio1 = sp->getEmbending("../static_data/anselme.wav");
    vector<float> audio2  = sp->getEmbending("../static_data/anselme2.wav");
    
    std::cout << "similarity : " << cosineSimilarity(audio1, audio2) << endl;

    return 0;
}