//
// Created by Yasushi Sakita on 25/02/01.
//

#ifndef REALTIMECUTVAD_REALTIME_CUT_VAD_H
#define REALTIMECUTVAD_REALTIME_CUT_VAD_H


#pragma once
#include "c_wrapper.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

#define K_8_SAMPLE_RATE 8000
#define K_8_SPLIT_CHUNK_SIZE 80 // 0.01s
#define K_16_SAMPLE_RATE 16000
#define K_16_SPLIT_CHUNK_SIZE 160 // 0.01s
#define K_24_SAMPLE_RATE 24000
#define K_24_SPLIT_CHUNK_SIZE 240 // 0.01s
#define K_48_SAMPLE_RATE 48000
#define K_48_SPLIT_CHUNK_SIZE 480 // 0.01s

// 1 / 16000 * 512 = 0.032s 1VADあたりの秒数

// 音声活性検出の確率の閾値（喋り始め）
#define SAMPLE_VAD_START_DETECTION_PROBABILITY_THRESHOLD 0.7
// 音声活性検出の確率の閾値（喋り途中のVAD閾値）
#define SAMPLE_VAD_END_DETECTION_PROBABILITY_THRESHOLD 0.7
// 音声開始の判断におけるVAD True判定の割合の閾値
#define SAMPLE_VOICE_START_VAD_TRUE_RATIO_THRESHOLD 0.8
// 音声終了の判断におけるVAD False判定の割合の閾値
#define SAMPLE_VOICE_END_VAD_FALSE_RATIO_THRESHOLD 0.95
// 音声開始を判断する音声ポイントの数
#define SAMPLE_VOICE_START_FRAME_COUNT_THRESHOLD 10 // 0.032s * 10 = 0.32s
// 音声終了を判断する音声ポイントの数
#define SAMPLE_VOICE_END_FRAME_COUNT_THRESHOLD 57 // 0.032s * 57 = 1.792s


enum SAMPLE_RATE {
    K_8 = 0,
    K_16 = 1,
    K_24 = 2,
    K_48 = 3,
};

enum SILERO_VER {
    V4 = 0,
    V5 = 1,
};

class RealTimeCutVAD {
public:
    RealTimeCutVAD();

    ~RealTimeCutVAD();

    void setCallback(
            void* instance,
            VoiceStartCallback voice_start_cb,
            VoiceEndCallback voice_end_cb) {
        context = instance;
        voice_start_callback = voice_start_cb;
        voice_end_callback = voice_end_cb;
    }

    void setSampleRate(SAMPLE_RATE choice_sample_rate);
    void setThreshold(float vad_start_detection_probability, float vad_end_detection_probability, float voice_start_vad_true_ratio, float voice_end_vad_false_ratio, int voice_start_frame_count, int voice_end_frame_count);

    void setModel(SILERO_VER ver, const std::string& model_path);

    void process(std::vector<float_t>& input_audio_data);

    class ISileroVAD;

private:
    SAMPLE_RATE choice_sample_rate;
    float vad_start_detection_probability_threshold;
    float vad_end_detection_probability_threshold;
    float voice_start_vad_true_ratio_threshold;
    float voice_end_vad_false_ratio_threshold;
    int voice_start_frame_count_threshold;
    int voice_end_frame_count_threshold;

    class ProcessImpl;
    ProcessImpl* pImpl;

    class AlgorithmImpl;
    AlgorithmImpl* aImpl;

    ISileroVAD* sImpl;

    void algorithm(float vadProbability, const std::vector<float_t>& sample);

    void* context;
    VoiceStartCallback voice_start_callback{};
    VoiceEndCallback voice_end_callback{};

};

#endif //REALTIMECUTVAD_REALTIME_CUT_VAD_H
