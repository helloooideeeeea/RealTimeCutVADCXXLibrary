#include <iostream>
#include <vector>
#include <sndfile.h>
#include <thread>
#include <fstream>
#include <thread>
#include "realtime_cut_vad.h"

std::string time_str() {
    auto now = std::chrono::system_clock::now(); // 現在時刻を取得
    auto now_as_time_t = std::chrono::system_clock::to_time_t(now); // time_t 形式に変換
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    long duration = value.count() % 1000; // ミリ秒部分を取得

    std::tm tm = *std::localtime(&now_as_time_t); // std::tm に変換
    char buffer[32]; // バッファを大きめにとる
    std::strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &tm); // 年月日時分秒のフォーマット

    std::ostringstream oss;
    oss << buffer << std::setw(3) << std::setfill('0') << duration; // ミリ秒を追加
    return oss.str();
}

void voiceStartCallback(void* context)
{
    std::cout << "録音開始" << std::endl;
}

void voiceEndCallback(void* context, const uint8_t* wav_data, size_t wav_size)
{
    std::cout << "録音終了" << std::endl;
    auto filename = std::string(PROJECT_ROOT_DIR)+"raw_"+time_str()+".wav";
    std::ofstream outfile(filename, std::ios::binary);
    outfile.write((const char*)&wav_data[0], wav_size);
    outfile.close();
    std::cout << "部分的な録音音声ファイル:" << filename << " を保存" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <onnx model file>" << " <Sample Rate>" << " <wav file>" << std::endl;
        return 1;
    }

    SF_INFO sfinfo;
    SNDFILE *infile = sf_open(argv[3], SFM_READ, &sfinfo);
    if (!infile) {
        std::cerr << "Failed to open file: " << argv[3] << std::endl;
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

    RealTimeCutVAD *vad = new RealTimeCutVAD();

    switch (std::atoi(argv[2])) {
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
            return 1;
    }

    vad->setCallback(
            NULL,
            voiceStartCallback,
            voiceEndCallback
    );

    vad->setModel(V4, std::string(argv[1]));
    vad->process(input);

    sf_close(infile);
    delete vad;
    return 0;
}