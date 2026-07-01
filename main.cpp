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
    vector<float> normal_audio = sp->getEmbending("voice.wav");
    vector<float> cleaned_audio  = sp->getEmbending("cleaned_voice.wav");

    vector<float> real_audio = sp->getEmbending("../static_data/voice1.wav");
    vector<float> cleaned_real_audio = sp->getEmbending("cleaned_voice1.wav");
   
    cout << cosineSimilarity(normal_audio, cleaned_audio) << endl;
    cout << cosineSimilarity(real_audio, cleaned_audio) << endl;
    cout << cosineSimilarity(real_audio, normal_audio) << endl;
    cout << cosineSimilarity(cleaned_real_audio, cleaned_audio) << endl;
    cout << cosineSimilarity(cleaned_real_audio, normal_audio) << endl;

    return 0;
}