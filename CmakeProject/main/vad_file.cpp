#include <iostream>
#include <vector>
#include <sndfile.h>
#include <thread>
#include <fstream>
#include <thread>
#include <filesystem>
#include <memory>
#include "realtime_cut_vad.h"

std::vector<uint8_t> pcm_data;

std::string time_str() {
    auto now = std::chrono::system_clock::now(); // Get current time
    auto now_as_time_t = std::chrono::system_clock::to_time_t(now); // Convert to time_t
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    long duration = value.count() % 1000; // Extract milliseconds

    std::tm tm = *std::localtime(&now_as_time_t); // Convert to std::tm
    char buffer[32]; // Sufficient buffer size
    std::strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &tm); // Format: YYYYMMDDHHMMSS

    std::ostringstream oss;
    oss << buffer << std::setw(3) << std::setfill('0') << duration; // Append milliseconds
    return oss.str();
}

std::vector<uint8_t> bytesToWavData(const std::vector<uint8_t>& pcmData, int sampleRate) {
    std::vector<uint8_t> wavData;

    uint32_t data_size = static_cast<uint32_t>(pcmData.size());
    uint16_t num_channels = 1;    // mono
    uint16_t bits_per_sample = 32; // 32-bit float
    uint32_t total_size = data_size + 44; // 44 bytes for standard WAV header
    uint32_t chunk_size = total_size - 8;
    uint32_t byte_rate = sampleRate * num_channels * (bits_per_sample / 8);
    uint16_t block_align = num_channels * (bits_per_sample / 8);

    wavData.reserve(total_size);

    // Write the "RIFF" header.
    wavData.insert(wavData.end(), {'R', 'I', 'F', 'F'});
    wavData.insert(wavData.end(),
                   reinterpret_cast<const uint8_t*>(&chunk_size),
                   reinterpret_cast<const uint8_t*>(&chunk_size) + sizeof(chunk_size));
    wavData.insert(wavData.end(), {'W', 'A', 'V', 'E'});

    // Write the "fmt " subchunk.
    wavData.insert(wavData.end(), {'f', 'm', 't', ' '});
    uint32_t fmt_chunk_size = 16;
    wavData.insert(wavData.end(),
                   reinterpret_cast<const uint8_t*>(&fmt_chunk_size),
                   reinterpret_cast<const uint8_t*>(&fmt_chunk_size) + sizeof(fmt_chunk_size));
    uint16_t audio_format = 3; // 3 indicates IEEE float format.
    wavData.insert(wavData.end(),
                   reinterpret_cast<const uint8_t*>(&audio_format),
                   reinterpret_cast<const uint8_t*>(&audio_format) + sizeof(audio_format));
    wavData.insert(wavData.end(),
                   reinterpret_cast<const uint8_t*>(&num_channels),
                   reinterpret_cast<const uint8_t*>(&num_channels) + sizeof(num_channels));
    wavData.insert(wavData.end(),
                   reinterpret_cast<const uint8_t*>(&sampleRate),
                   reinterpret_cast<const uint8_t*>(&sampleRate) + sizeof(sampleRate));
    wavData.insert(wavData.end(),
                   reinterpret_cast<const uint8_t*>(&byte_rate),
                   reinterpret_cast<const uint8_t*>(&byte_rate) + sizeof(byte_rate));
    wavData.insert(wavData.end(),
                   reinterpret_cast<const uint8_t*>(&block_align),
                   reinterpret_cast<const uint8_t*>(&block_align) + sizeof(block_align));
    wavData.insert(wavData.end(),
                   reinterpret_cast<const uint8_t*>(&bits_per_sample),
                   reinterpret_cast<const uint8_t*>(&bits_per_sample) + sizeof(bits_per_sample));

    // Write the "data" subchunk header.
    wavData.insert(wavData.end(), {'d', 'a', 't', 'a'});
    wavData.insert(wavData.end(),
                   reinterpret_cast<const uint8_t*>(&data_size),
                   reinterpret_cast<const uint8_t*>(&data_size) + sizeof(data_size));

    // Append the raw PCM data.
    wavData.insert(wavData.end(), pcmData.begin(), pcmData.end());

    return wavData;
}

void voiceStartCallback(void* context)
{
    std::cout << "Voice recording started." << std::endl;
    pcm_data.clear();
}

void voiceEndCallback(void* context, const uint8_t* wav_data, size_t wav_size)
{
    std::cout << "Recording finished." << std::endl;
    std::filesystem::path filename = std::filesystem::path(PROJECT_SOURCE_DIR) / ("raw_" + time_str() + ".wav");

    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Failed to open file for writing: " << filename << std::endl;
        return;
    }

    outfile.write(reinterpret_cast<const char*>(wav_data), wav_size);
    outfile.close();

    std::cout << "Partial recorded audio file saved: " << filename << std::endl;

    // pcm_data
    std::cout << "start pcm_data save" << std::endl;
    int sampleRate = 16000;
    std::vector<uint8_t> wavFileData = bytesToWavData(pcm_data, sampleRate);
    pcm_data.clear();

    std::filesystem::path pcm_filename = std::filesystem::path(PROJECT_SOURCE_DIR) / ("pcm_" + time_str() + ".wav");

    std::ofstream pcm_outfile(pcm_filename, std::ios::binary);
    if (!pcm_outfile) {
        std::cerr << "Error: Failed to open file for writing: " << pcm_filename << std::endl;
        return;
    }
    pcm_outfile.write(reinterpret_cast<const char*>(wavFileData.data()), wavFileData.size());
    pcm_outfile.close();

    std::cout << "pcm file saved: " << pcm_filename << std::endl;

}

void voiceDidContinueCallback(void* context, const uint8_t* pcm_float_data, size_t data_size)
{
    std::cout << "Recording ongoing..." << std::endl;
    pcm_data.insert(pcm_data.end(), pcm_float_data, pcm_float_data + data_size);
}

int main(int argc, char *argv[]) {
    std::cout << "Current directory: " << std::filesystem::current_path() << std::endl;
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <onnx model file>" << " <Silero Version v4 or v5>" << " <Sample Rate>" << " <wav file>" << std::endl;
        return 1;
    }

    std::string model_path = argv[1];
    std::string model_version = argv[2];
    int sample_rate = std::atoi(argv[3]);
    char* wave_path = argv[4];

    SF_INFO sfinfo;
    SNDFILE *infile = sf_open(wave_path, SFM_READ, &sfinfo);
    if (!infile) {
        std::cerr << "Failed to open file: " << wave_path << std::endl;
        return 1;
    }

    std::vector<float> input(sfinfo.frames * sfinfo.channels);
    sf_count_t num_items = sfinfo.frames * sfinfo.channels;

    sf_count_t num_read = sf_read_float(infile, input.data(), num_items);
    if (num_read != num_items) {
        std::cerr << "Failed to read all frames" << std::endl;
        sf_close(infile);
        return 1;
    }

    std::cout << "floats num " << input.size() << std::endl;

    std::unique_ptr<RealTimeCutVAD> vad = std::make_unique<RealTimeCutVAD>();

    if (model_version == "v4") {
        vad->setModel(V4, std::string(model_path));
    } else if (model_version == "v5") {
        vad->setModel(V5, std::string(model_path));
    } else {
        std::cerr << "Invalid model version: " << model_version << std::endl;
        return 1;
    }

    switch (sample_rate) {
        case 8000:
            vad->setSampleRate(K_8);
            break;
        case 16000:
            vad->setSampleRate(K_16);
            break;
        case 24000:
            vad->setSampleRate(K_24);
            break;
        case 48000:
            vad->setSampleRate(K_48);
            break;
        default:
            std::cerr << "Invalid sample rate: " << sample_rate << std::endl;
            return 1;
    }

    vad->setCallback(
            NULL,
            voiceStartCallback,
            voiceEndCallback,
            voiceDidContinueCallback
    );

    vad->process(input);

    sf_close(infile);
    return 0;
}