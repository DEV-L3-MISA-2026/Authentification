#include <rnnoise.h>
#include <AudioProcessor.h>
#include <SpeechProcessor.h>
#include <fstream>
#include <iostream>
using namespace std;

int main() {
    auto audioprocessor = AudioProcessor::getInstance();
    vector<float> audio = audioprocessor->loadAudioForCleaning("voice.wav");
    vector<float> cleaned = audioprocessor->cleanAudioRNNoise(audio);
    
    // saving the cleaned version in another file
    audioprocessor->saveAudio("cleaned_voice.wav", cleaned, 16000);
    cout << "Succesfully saved the audio !";    
    return 0;
}