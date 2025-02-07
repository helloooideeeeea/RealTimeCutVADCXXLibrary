//
// Created by Yasushi Sakita on 25/02/01.
//

#include "realtime_cut_vad.h"
#include "process.h"
#include "algorithm.h"
#include "silero.h"
#include "misc.h"


RealTimeCutVAD::RealTimeCutVAD() {
    pImpl = new ProcessImpl;

    pImpl->apm = webrtc::AudioProcessingBuilder().Create();
    webrtc::AudioProcessing::Config config;

    config.echo_canceller.enabled = false;

    config.gain_controller1.enabled = true;
    config.gain_controller1.mode = webrtc::AudioProcessing::Config::GainController1::kAdaptiveDigital;
    config.gain_controller2.enabled = true;
    config.high_pass_filter.enabled = true;
    config.noise_suppression.enabled = true;
    config.transient_suppression.enabled = true;
    config.voice_detection.enabled = false;
    pImpl->apm->ApplyConfig(config);
    pImpl->apm->Initialize();

    aImpl = new AlgorithmImpl;
    aImpl->resetVariables();

    this->vad_start_detection_probability_threshold = SAMPLE_VAD_START_DETECTION_PROBABILITY_THRESHOLD;
    this->vad_end_detection_probability_threshold = SAMPLE_VAD_END_DETECTION_PROBABILITY_THRESHOLD;
    this->voice_start_vad_true_ratio_threshold = SAMPLE_VOICE_START_VAD_TRUE_RATIO_THRESHOLD;
    this->voice_end_vad_false_ratio_threshold = SAMPLE_VOICE_END_VAD_FALSE_RATIO_THRESHOLD;
    this->voice_start_frame_count_threshold = SAMPLE_VOICE_START_FRAME_COUNT_THRESHOLD;
    this->voice_end_frame_count_threshold = SAMPLE_VOICE_END_FRAME_COUNT_THRESHOLD;

}

RealTimeCutVAD::~RealTimeCutVAD() {
    delete pImpl->apm;
    delete pImpl;
    delete aImpl;
    delete sImpl;
}

void RealTimeCutVAD::setSampleRate(SAMPLE_RATE choice_sample_rate_param) {
    this->choice_sample_rate = choice_sample_rate_param;
    int sampleRate = getSampleRateValue(this->choice_sample_rate);
    webrtc::StreamConfig inputConfig(sampleRate, 1);
    webrtc::StreamConfig outputConfig(K_16_SAMPLE_RATE, 1);
    pImpl->inputConfig = inputConfig;
    pImpl->outputConfig = outputConfig;
    switch (this->choice_sample_rate) {
        case K_8:
            pImpl->input_audio_split_chunk_size = K_8_SPLIT_CHUNK_SIZE;
            pImpl->apm_sample_size = K_8_SPLIT_CHUNK_SIZE * K_16_SAMPLE_RATE / K_8_SAMPLE_RATE;
            break;
        case K_16:
            pImpl->input_audio_split_chunk_size = K_16_SPLIT_CHUNK_SIZE;
            pImpl->apm_sample_size = K_16_SPLIT_CHUNK_SIZE;
            break;
        case K_24:
            pImpl->input_audio_split_chunk_size = K_24_SPLIT_CHUNK_SIZE;
            pImpl->apm_sample_size = K_24_SPLIT_CHUNK_SIZE * K_16_SAMPLE_RATE / K_24_SAMPLE_RATE;
            break;
        case K_48:
            pImpl->input_audio_split_chunk_size = K_48_SPLIT_CHUNK_SIZE;
            pImpl->apm_sample_size = K_48_SPLIT_CHUNK_SIZE * K_16_SAMPLE_RATE / K_48_SAMPLE_RATE;
            break;
        default:
            throw std::runtime_error("Choose sample rate must be 8khz or 16khz or 24khz or 48khz.");
    }
}


void RealTimeCutVAD::setThreshold(float vad_start_detection_probability, float vad_end_detection_probability, float voice_start_vad_true_ratio, float voice_end_vad_false_ratio, int voice_start_frame_count, int voice_end_frame_count) {
    this->vad_start_detection_probability_threshold = vad_start_detection_probability;
    this->vad_end_detection_probability_threshold = vad_end_detection_probability;
    this->voice_start_vad_true_ratio_threshold = voice_start_vad_true_ratio;
    this->voice_end_vad_false_ratio_threshold = voice_end_vad_false_ratio;
    this->voice_start_frame_count_threshold = voice_start_frame_count;
    this->voice_end_frame_count_threshold = voice_end_frame_count;
}

void RealTimeCutVAD::setModel(SILERO_VER ver, const std::string& model_path){
    // 既存のオブジェクトを削除
    if (sImpl) {
        delete sImpl;
        sImpl = nullptr;
    }

    switch (ver) {
        case V4:
            sImpl = new SileroV4;
            break;
        case V5:
            sImpl = new SileroV5;
            break;
        default:
            throw std::invalid_argument("Invalid SILERO_VER specified");
    }

    sImpl->init_onnx_model(model_path);
}

