#include <rnnoise.h>
#include <AudioProcessor.h>
#include <fstream>
using namespace std;
int main() {
    auto audioprocessor = AudioProcessor::getInstance();
    vector<float> audio = audioprocessor->loadAudioForCleaning("voice.wav");
    vector<float> cleaned = audioprocessor->cleanAudioRNNoise(audio);

    // saving the cleaned version in another file
    ofstream fs("voice_cleaned.wav", ios::binary);

    return 0;
}