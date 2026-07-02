// vad_extract_speech.cpp
//
// Lit un fichier WAV, utilise Silero VAD (v5, ONNX Runtime) pour détecter
// les segments de parole, puis les concatène dans un nouveau fichier WAV
// (le silence est retiré).
//
// Dépendances :
//   - onnxruntime (C++ API)
//   - libsndfile (API C : sndfile.h)
//
// Compilation (exemple Linux) :
//   g++ -std=c++17 vad_extract_speech.cpp -o vad_extract_speech \
//       -I/path/to/onnxruntime/include -L/path/to/onnxruntime/lib -lonnxruntime \
//       -lsndfile -Wl,-rpath,/path/to/onnxruntime/lib
//
// Utilisation :
//   ./vad_extract_speech input.wav silero_vad.onnx output_speech_only.wav
//
// Le WAV d'entrée doit être mono. S'il n'est pas à 16000 Hz, adapte
// SAMPLE_RATE et re-échantillonne au préalable (non géré ici).

#include <onnxruntime/onnxruntime_cxx_api.h>
#include <sndfile.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {

constexpr int SAMPLE_RATE = 16000;      // Silero VAD attend 16000 ou 8000 Hz
constexpr int WINDOW_SAMPLES = 512;     // taille de chunk attendue par le modèle (32 ms @16kHz)
constexpr float THRESHOLD = 0.5f;       // seuil probabilité -> "parole"

// Paramètres de lissage temporel (en millisecondes), classiques pour Silero VAD
constexpr int MIN_SPEECH_MS = 250;      // ignore les segments de parole trop courts
constexpr int MIN_SILENCE_MS = 300;     // silence minimal pour considérer la parole terminée
constexpr int SPEECH_PAD_MS = 150;      // marge ajoutée avant/après chaque segment détecté

} // namespace

// ------------------------------------------------------------------------
// Wrapper autour du modèle ONNX Silero VAD (v5 : inputs input/state/sr,
// outputs output/stateN)
// ------------------------------------------------------------------------
class SileroVAD {
public:
    SileroVAD(const std::string& model_path, int sample_rate = SAMPLE_RATE)
        : env_(ORT_LOGGING_LEVEL_WARNING, "silero_vad"), sample_rate_(sample_rate) {

        Ort::SessionOptions opts;
        opts.SetIntraOpNumThreads(1);
        opts.SetInterOpNumThreads(1);
        session_ = std::make_unique<Ort::Session>(env_, model_path.c_str(), opts);

        Ort::AllocatorWithDefaultOptions allocator;

        size_t n_in = session_->GetInputCount();
        for (size_t i = 0; i < n_in; ++i) {
            auto name = session_->GetInputNameAllocated(i, allocator);
            input_names_storage_.emplace_back(name.get());
        }
        for (auto& s : input_names_storage_) input_names_.push_back(s.c_str());

        size_t n_out = session_->GetOutputCount();
        for (size_t i = 0; i < n_out; ++i) {
            auto name = session_->GetOutputNameAllocated(i, allocator);
            output_names_storage_.emplace_back(name.get());
        }
        for (auto& s : output_names_storage_) output_names_.push_back(s.c_str());

        std::cerr << "Modele charge. Inputs: ";
        for (auto& n : input_names_storage_) std::cerr << n << " ";
        std::cerr << "| Outputs: ";
        for (auto& n : output_names_storage_) std::cerr << n << " ";
        std::cerr << std::endl;

        resetState();
    }

    void resetState() { state_.assign(2 * 1 * 128, 0.0f); }

    // Traite un chunk de exactement WINDOW_SAMPLES échantillons float32 [-1,1]
    // et retourne la probabilité de parole (0..1)
    float process(const std::vector<float>& chunk) {
        Ort::MemoryInfo mem_info =
            Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        std::vector<int64_t> input_shape = {1, (int64_t)chunk.size()};
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            mem_info, const_cast<float*>(chunk.data()), chunk.size(),
            input_shape.data(), input_shape.size());

        std::vector<int64_t> state_shape = {2, 1, 128};
        Ort::Value state_tensor = Ort::Value::CreateTensor<float>(
            mem_info, state_.data(), state_.size(), state_shape.data(),
            state_shape.size());

        std::vector<int64_t> sr_shape = {1};
        int64_t sr = sample_rate_;
        Ort::Value sr_tensor = Ort::Value::CreateTensor<int64_t>(
            mem_info, &sr, 1, sr_shape.data(), sr_shape.size());

        std::vector<Ort::Value> inputs;
        inputs.push_back(std::move(input_tensor));
        inputs.push_back(std::move(state_tensor));
        inputs.push_back(std::move(sr_tensor));

        auto outputs = session_->Run(Ort::RunOptions{nullptr}, input_names_.data(),
                                      inputs.data(), inputs.size(),
                                      output_names_.data(), output_names_.size());

        float prob = outputs[0].GetTensorMutableData<float>()[0];

        float* new_state = outputs[1].GetTensorMutableData<float>();
        std::copy(new_state, new_state + state_.size(), state_.begin());

        return prob;
    }

private:
    Ort::Env env_;
    std::unique_ptr<Ort::Session> session_;
    std::vector<std::string> input_names_storage_;
    std::vector<std::string> output_names_storage_;
    std::vector<const char*> input_names_;
    std::vector<const char*> output_names_;
    std::vector<float> state_;
    int sample_rate_;
};

struct Segment {
    size_t start_sample;
    size_t end_sample; // exclusif
};

// ------------------------------------------------------------------------
// Passe le signal complet dans le VAD chunk par chunk, et applique un
// lissage temporel (durée min de parole / silence + padding) pour produire
// des segments [start, end) propres, plutôt que des probabilités brutes
// instables chunk par chunk.
// ------------------------------------------------------------------------
std::vector<Segment> detectSpeechSegments(SileroVAD& vad,
                                           const std::vector<float>& samples,
                                           int sample_rate) {
    vad.resetState();

    const size_t n = samples.size();
    const size_t min_speech_samples = (size_t)(MIN_SPEECH_MS * sample_rate / 1000);
    const size_t min_silence_samples = (size_t)(MIN_SILENCE_MS * sample_rate / 1000);
    const size_t pad_samples = (size_t)(SPEECH_PAD_MS * sample_rate / 1000);

    std::vector<Segment> raw_segments;
    bool in_speech = false;
    size_t speech_start = 0;
    size_t silence_run = 0;

    for (size_t offset = 0; offset + WINDOW_SAMPLES <= n; offset += WINDOW_SAMPLES) {
        std::vector<float> chunk(samples.begin() + offset,
                                  samples.begin() + offset + WINDOW_SAMPLES);
        float prob = vad.process(chunk);
        bool is_speech = prob >= THRESHOLD;

        if (is_speech) {
            if (!in_speech) {
                in_speech = true;
                speech_start = offset;
            }
            silence_run = 0;
        } else if (in_speech) {
            silence_run += WINDOW_SAMPLES;
            if (silence_run >= min_silence_samples) {
                size_t speech_end = offset + WINDOW_SAMPLES - silence_run;
                if (speech_end > speech_start &&
                    speech_end - speech_start >= min_speech_samples) {
                    raw_segments.push_back({speech_start, speech_end});
                }
                in_speech = false;
                silence_run = 0;
            }
        }
    }
    
    // segment encore ouvert à la fin du fichier
    if (in_speech) {
        size_t speech_end = n;
        if (speech_end > speech_start &&
            speech_end - speech_start >= min_speech_samples) {
            raw_segments.push_back({speech_start, speech_end});
        }
    }

    // Ajoute un padding avant/après chaque segment, puis fusionne les
    // segments qui se chevauchent suite au padding.
    std::vector<Segment> padded;
    for (auto& s : raw_segments) {
        size_t start = (s.start_sample > pad_samples) ? s.start_sample - pad_samples : 0;
        size_t end = std::min(n, s.end_sample + pad_samples);
        padded.push_back({start, end});
    }

    std::vector<Segment> merged;
    for (auto& s : padded) {
        if (!merged.empty() && s.start_sample <= merged.back().end_sample) {
            merged.back().end_sample = std::max(merged.back().end_sample, s.end_sample);
        } else {
            merged.push_back(s);
        }
    }

    return merged;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <input.wav> <silero_vad.onnx> <output.wav>" << std::endl;
        return 1;
    }

    std::string input_wav = argv[1];
    std::string model_path = argv[2];
    std::string output_wav = argv[3];

    // --- Lecture du WAV avec l'API C de libsndfile ---
    SF_INFO in_info;
    memset(&in_info, 0, sizeof(in_info)); // doit rester a zero avant sf_open en lecture

    SNDFILE* infile = sf_open(input_wav.c_str(), SFM_READ, &in_info);
    if (!infile) {
        std::cerr << "Erreur ouverture " << input_wav << ": " << sf_strerror(nullptr)
                  << std::endl;
        return 1;
    }

    if (in_info.channels != 1) {
        std::cerr << "Attention: le fichier a " << in_info.channels
                  << " canaux. Ce programme suppose du mono. "
                  << "Convertis en mono au prealable (ex: sox/ffmpeg)." << std::endl;
        sf_close(infile);
        return 1;
    }

    if (in_info.samplerate != SAMPLE_RATE) {
        std::cerr << "Attention: sample rate = " << in_info.samplerate
                  << " Hz, attendu " << SAMPLE_RATE
                  << " Hz. Re-echantillonne le fichier au prealable "
                  << "(ex: sox input.wav -r 16000 input_16k.wav)." << std::endl;
        sf_close(infile);
        return 1;
    }

    std::vector<float> samples((size_t)in_info.frames);
    sf_count_t read = sf_readf_float(infile, samples.data(), in_info.frames);
    samples.resize((size_t)read); // au cas ou moins de frames lues que prevu
    sf_close(infile);

    std::cerr << "Fichier charge: " << samples.size() << " echantillons ("
              << (double)samples.size() / SAMPLE_RATE << " s)" << std::endl;

    // --- VAD ---
    SileroVAD vad(model_path, SAMPLE_RATE);
    std::vector<Segment> segments = detectSpeechSegments(vad, samples, SAMPLE_RATE);

    std::cerr << "Segments de parole detectes: " << segments.size() << std::endl;
    for (auto& s : segments) {
        std::cerr << "  [" << (double)s.start_sample / SAMPLE_RATE << "s -> "
                  << (double)s.end_sample / SAMPLE_RATE << "s]" << std::endl;
    }

    // --- Concatenation des segments de parole ---
    std::vector<float> speech_only;
    speech_only.reserve(samples.size());
    for (auto& s : segments) {
        std::cout << "il y a un segment ici \n";
        speech_only.insert(speech_only.end(), samples.begin() + s.start_sample,
                            samples.begin() + s.end_sample);
    }

    std::cerr << "Duree finale (parole uniquement): "
              << (double)speech_only.size() / SAMPLE_RATE << " s (sur "
              << (double)samples.size() / SAMPLE_RATE << " s d'origine)" << std::endl;

    // --- Ecriture du WAV de sortie avec l'API C de libsndfile ---
    SF_INFO out_info;
    memset(&out_info, 0, sizeof(out_info));
    out_info.samplerate = SAMPLE_RATE;
    out_info.channels = 1;
    out_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

    SNDFILE* outfile = sf_open(output_wav.c_str(), SFM_WRITE, &out_info);
    if (!outfile) {
        std::cerr << "Erreur creation " << output_wav << ": " << sf_strerror(nullptr)
                  << std::endl;
        return 1;
    }

    sf_count_t written = sf_writef_float(outfile, speech_only.data(),
                                          (sf_count_t)speech_only.size());
    if (written != (sf_count_t)speech_only.size()) {
        std::cerr << "Attention: seulement " << written << "/" << speech_only.size()
                  << " echantillons ecrits." << std::endl;
    }

    sf_close(outfile);

    std::cerr << "Ecrit: " << output_wav << std::endl;

    return 0;
}