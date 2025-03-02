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
    std::unique_ptr<ProcessImpl> pImpl;

    class AlgorithmImpl;
    std::unique_ptr<AlgorithmImpl> aImpl;

    std::unique_ptr<ISileroVAD> sImpl;

    void algorithm(float vadProbability, const std::vector<float_t>& sample);

    void* context;
    VoiceStartCallback voice_start_callback{};
    VoiceEndCallback voice_end_callback{};

};

#endif //REALTIMECUTVAD_REALTIME_CUT_VAD_H
