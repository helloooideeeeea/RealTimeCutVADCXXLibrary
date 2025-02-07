//
// Created by Yasushi Sakita on 25/02/01.
//

#include "realtime_cut_vad.h"
#include "algorithm.h"
#include "silero.h"
#include "tool.h"


void RealTimeCutVAD::algorithm(float vadProbability, const std::vector<float_t>& sample) {

    if (!aImpl->recording) {
        bool is_speech = vadProbability > this->vad_start_detection_probability_threshold;
//        std::cout << is_speech << std::endl;

        aImpl->pre_vad_list.push_back(is_speech);
        aImpl->pre_audio_buffer.insert(aImpl->pre_audio_buffer.end(), sample.begin(), sample.end());
        size_t desired_size = sample.size() * this->voice_start_frame_count_threshold * 4;
        if (aImpl->pre_audio_buffer.size() >= desired_size) {
            // 入力されたsample分のみ、PreAudioBufferから削除する。つまり、0.32s * 4 - 0.064s = 約1.216sのPreAudioBufferは確保されたままである
            aImpl->pre_audio_buffer.erase(aImpl->pre_audio_buffer.begin(), aImpl->pre_audio_buffer.begin() + sample.size());
        }

//        std::cout << "pre_audio_buffer.size :" << aImpl->pre_audio_buffer.size() << std::endl;

        if (aImpl->pre_vad_list.size() == this->voice_start_frame_count_threshold) {
            if (aImpl->trueProbability(aImpl->pre_vad_list, this->voice_start_vad_true_ratio_threshold)) {
                aImpl->audio_buffer.assign(aImpl->pre_audio_buffer.begin(), aImpl->pre_audio_buffer.end());

                aImpl->pre_audio_buffer.clear();
                aImpl->pre_vad_list.clear();
                aImpl->recording = true;
                if (voice_start_callback) {
                    voice_start_callback(this->context);
                }

                std::cout << "Voice recording start!" << std::endl;

            } else {
                aImpl->pre_vad_list.erase(aImpl->pre_vad_list.begin());
            }
        }

    } else {
        bool is_speech = vadProbability > this->vad_end_detection_probability_threshold;
        aImpl->start_vad_list.push_back(is_speech);
        aImpl->audio_buffer.insert(aImpl->audio_buffer.end(), sample.begin(), sample.end());

        if (aImpl->start_vad_list.size() == this->voice_end_frame_count_threshold) {
            if (aImpl->falseProbability(aImpl->start_vad_list, this->voice_end_vad_false_ratio_threshold)) {

                // audioの秒数
                // float audio_seconds = aImpl->audio_buffer.size() / K_16_SAMPLE_RATE;
                if (voice_end_callback) {
                    std::vector<uint8_t> wav_data = framesToWavData(aImpl->audio_buffer, K_16_SAMPLE_RATE);
                    voice_end_callback(this->context, wav_data.data(), wav_data.size());
                }

                aImpl->resetVariables();
                sImpl->reset_hidden_layer_value();

                std::cout << "Voice recording end!" << std::endl;

            } else {
                aImpl->start_vad_list.erase(aImpl->start_vad_list.begin());
            }
        }
    }
}

void RealTimeCutVAD::AlgorithmImpl::resetVariables() {
    pre_vad_list.clear();
    start_vad_list.clear();
    audio_buffer.clear();
    pre_audio_buffer.clear();
    recording = false;
}

bool RealTimeCutVAD::AlgorithmImpl::trueProbability(const std::vector<bool>& list, float prob) {
    int trueCount = std::count(list.begin(), list.end(), true);
    return (trueCount / static_cast<float>(list.size())) > prob;
}

bool RealTimeCutVAD::AlgorithmImpl::falseProbability(const std::vector<bool>& list, float prob) {
    int falseCount = std::count(list.begin(), list.end(), false);
    return (falseCount / static_cast<float>(list.size())) > prob;
}

