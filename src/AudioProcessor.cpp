#include "../include/AudioProcessor.h"
#include <sndfile.h>
#include <samplerate.h>
#include <iostream>

std::shared_ptr<AudioProcessor> AudioProcessor::instance = nullptr;
AudioProcessor::AudioProcessor() {

}

std::vector<float> AudioProcessor::resample(const std::vector<float>& input, int inputRate, int outputRate) 
{
    if (inputRate == outputRate) return input;

    double ratio = (double)outputRate / inputRate;
    std::vector<float> output(
        input.size() * ratio + 1
    );
    SRC_DATA data{};
    data.data_in = input.data();
    data.input_frames = input.size();

    data.data_out = output.data();
    data.output_frames = output.size();

    data.src_ratio = ratio;
    data.end_of_input = 1;

    int err = src_simple(&data, SRC_SINC_BEST_QUALITY, 1);

    if (err)
        throw std::runtime_error(src_strerror(err));

    output.resize(data.output_frames_gen);
    return output;
}

std::vector<float> AudioProcessor::stereoToMono(const std::vector<float>& input) {
    std::vector<float> mono(input.size()/2);
    for (int i = 0; i < input.size()/2; ++i)
        mono[i] = 0.5f * (input[2*i] + input[2 * i + 1]); // mean of consecutive frame
    return mono;
}

std::vector<float> AudioProcessor::loadAudio(const std::string& path) {

    SF_INFO info{};
    SNDFILE* file = sf_open(path.c_str(), SFM_READ, &info);

    std::vector<float> interleaved(info.frames * info.channels);

    sf_read_float(file, interleaved.data(), interleaved.size());

    sf_close(file);

    std::vector<float> mono(info.frames);

    if (info.channels == 1)
        mono = std::move(interleaved);
    else if (info.channels == 2)
        mono = this->stereoToMono(interleaved);
    else
        throw std::runtime_error("Unsupported number of channels.");

    // resampling to 16MHz
    mono = this->resample(
        mono,
        info.samplerate,
        16000
    );
    sf_close(file);
    return mono;
}

std::shared_ptr<AudioProcessor> AudioProcessor::getInstance() {
    if (instance == nullptr)
        instance = std::shared_ptr<AudioProcessor>();
    return instance;
}