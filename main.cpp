#include <rnnoise.h>
#include <AudioProcessor.h>
#include <SpeechProcessor.h>
#include <fstream>
#include <iostream>
#include <utility.h>
#include <Vad.h>
using namespace std;

int main() {
    
    auto audioprocessor = AudioProcessor::getInstance();
    SpeechProcessor::init("../model/speech_detection.onnx");
    auto sp = SpeechProcessor::getInstance();
    // vad 
    Vad::init("../model/silero_vad.onnx");
    auto vad = Vad::getInstance();
    
    vector<float> anselme = audioprocessor->loadAudio("anselme.wav");

    cout << "loaded the audio : " << anselme.size() << " bytes \n";
    
    vector<float> removed_silence = vad->removeSilence(anselme);
    cout << "removed silence size: " << removed_silence.size() << "\n";

    audioprocessor->saveAudio("sans_silence.wav", removed_silence, 16000);
    return 0;
}