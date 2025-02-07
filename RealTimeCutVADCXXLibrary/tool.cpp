//
// Created by Yasushi Sakita on 25/02/01.
//

#include "tool.h"

std::vector<uint8_t> framesToWavData(const std::vector<float_t>& audio_frames, int sampleRate) {

    std::vector<uint8_t> wav_data;

    // WAVヘッダーの作成
    uint32_t num_samples = audio_frames.size();
    uint16_t num_channels = 1; // モノラル
    uint16_t bits_per_sample = 32; // 32ビット浮動小数点

    // ヘッダー情報の計算
    uint32_t data_size = num_samples * num_channels * (bits_per_sample / 8);
    uint32_t total_size = data_size + 44;

    // WAVヘッダーの書き込み
    wav_data.reserve(total_size); // wav_dataのサイズを予め定義
    wav_data.insert(wav_data.end(), {'R', 'I', 'F', 'F'});
    uint32_t chunk_size = total_size - 8;
    wav_data.insert(wav_data.end(), reinterpret_cast<const uint8_t*>(&chunk_size), reinterpret_cast<const uint8_t*>(&chunk_size) + sizeof(chunk_size));
    wav_data.insert(wav_data.end(), {'W', 'A', 'V', 'E'});
    wav_data.insert(wav_data.end(), {'f', 'm', 't', ' '});
    uint32_t fmt_chunk_size = 16;
    wav_data.insert(wav_data.end(), reinterpret_cast<const uint8_t*>(&fmt_chunk_size), reinterpret_cast<const uint8_t*>(&fmt_chunk_size) + sizeof(fmt_chunk_size));
    uint16_t audio_format = 3; // IEEE浮動小数点数
    wav_data.insert(wav_data.end(), reinterpret_cast<const uint8_t*>(&audio_format), reinterpret_cast<const uint8_t*>(&audio_format) + sizeof(audio_format));
    wav_data.insert(wav_data.end(), reinterpret_cast<const uint8_t*>(&num_channels), reinterpret_cast<const uint8_t*>(&num_channels) + sizeof(num_channels));
    wav_data.insert(wav_data.end(), reinterpret_cast<const uint8_t*>(&sampleRate), reinterpret_cast<const uint8_t*>(&sampleRate) + sizeof(sampleRate));
    uint32_t byte_rate = sampleRate * num_channels * (bits_per_sample / 8);
    wav_data.insert(wav_data.end(), reinterpret_cast<const uint8_t*>(&byte_rate), reinterpret_cast<const uint8_t*>(&byte_rate) + sizeof(byte_rate));
    uint16_t block_align = num_channels * (bits_per_sample / 8);
    wav_data.insert(wav_data.end(), reinterpret_cast<const uint8_t*>(&block_align), reinterpret_cast<const uint8_t*>(&block_align) + sizeof(block_align));
    wav_data.insert(wav_data.end(), reinterpret_cast<const uint8_t*>(&bits_per_sample), reinterpret_cast<const uint8_t*>(&bits_per_sample) + sizeof(bits_per_sample));
    wav_data.insert(wav_data.end(), {'d', 'a', 't', 'a'});
    wav_data.insert(wav_data.end(), reinterpret_cast<const uint8_t*>(&data_size), reinterpret_cast<const uint8_t*>(&data_size) + sizeof(data_size));

    // 音声データの書き込み
    const uint8_t* audio_data = reinterpret_cast<const uint8_t*>(audio_frames.data());
    wav_data.insert(wav_data.end(), audio_data, audio_data + data_size);

    return wav_data;
}

