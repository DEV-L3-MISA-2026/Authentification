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

    vector<float> audio = audioprocessor->loadAudioForCleaning("../static_data/anselme.wav");
    vector<float> cleaned = audioprocessor->cleanAudioRNNoise(audio);
    
    // saving the cleaned version in another file
    audioprocessor->saveAudio("cleaned_anselme.wav", cleaned, 48000);

    vector<float> audio22 = audioprocessor->loadAudioForCleaning("../static_data/anselme2.wav");
    vector<float> cleaned2 = audioprocessor->cleanAudioRNNoise(audio22);
    
    // saving the cleaned version in another file
    audioprocessor->saveAudio("cleaned_anselme2.wav", cleaned2, 48000);

    auto sp = SpeechProcessor::getInstance();
    
    // testing the result
    vector<float> audio1 = sp->getEmbending("../static_data/anselme.wav");
    vector<float> audio2  = sp->getEmbending("../static_data/anselme2.wav");
    
    std::cout << "similarity : " << cosineSimilarity(audio1, audio2) << endl;

    return 0;
}