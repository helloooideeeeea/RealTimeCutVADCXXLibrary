//
// Created by Yasushi Sakita on 25/02/01.
//

#include "realtime_cut_vad.h"
#include "process.h"
#include "algorithm.h"
#include "silero.h"
#include "misc.h"


RealTimeCutVAD::RealTimeCutVAD() {
    pImpl = std::make_unique<ProcessImpl>();

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

    aImpl = std::make_unique<AlgorithmImpl>();
    aImpl->resetVariables();

}

RealTimeCutVAD::~RealTimeCutVAD() {
    pImpl.reset();
    aImpl.reset();
    sImpl.reset();
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
    /**
     * vad_start_detection_probability.. Threshold probability for detecting voice activity start (e.g., 0.7)
     * vad_end_detection_probability.. Threshold probability for detecting ongoing voice activity (e.g., 0.7)
     * voice_start_vad_true_ratio.. Threshold ratio of VAD true detections required to confirm voice start (e.g., 0.5)
     * voice_end_vad_false_ratio.. Threshold ratio of VAD false detections required to confirm voice end (e.g., 0.95)
     * voice_start_frame_count.. Number of VAD frames required to confirm voice start (e.g., 10 → 0.032s * 10 = 0.32s)
     * voice_end_frame_count.. Number of VAD frames required to confirm voice end (e.g., 57 → 0.032s * 57 = 1.824s)
     *
     * 1 / 16000 * 512 = 0.032s (Duration per VAD frame)
     */

    this->vad_start_detection_probability_threshold = vad_start_detection_probability;
    this->vad_end_detection_probability_threshold = vad_end_detection_probability;
    this->voice_start_vad_true_ratio_threshold = voice_start_vad_true_ratio;
    this->voice_end_vad_false_ratio_threshold = voice_end_vad_false_ratio;
    this->voice_start_frame_count_threshold = voice_start_frame_count;
    this->voice_end_frame_count_threshold = voice_end_frame_count;

    std::cout << "VAD Thresholds configured:\n"
              << "  Voice Activity Start Detection Probability: " << vad_start_detection_probability << "\n"
              << "  Ongoing Voice Activity Detection Probability: " << vad_end_detection_probability << "\n"
              << "  Required VAD True Ratio for Voice Start: " << voice_start_vad_true_ratio << "\n"
              << "  Required VAD False Ratio for Voice End: " << voice_end_vad_false_ratio << "\n"
              << "  Frames Required for Voice Start Detection: " << voice_start_frame_count
              << " (" << (voice_start_frame_count * 0.032) << " seconds)\n"
              << "  Frames Required for Voice End Detection: " << voice_end_frame_count
              << " (" << (voice_end_frame_count * 0.032) << " seconds)\n";
}

void RealTimeCutVAD::setModel(SILERO_VER ver, const std::string& model_path){
    // 既存のオブジェクトを削除
    if (sImpl) {
        sImpl.reset();
    }

    switch (ver) {
        case V4:
            sImpl = std::make_unique<SileroV4>();
            setThreshold(0.7f, 0.7f, 0.8f, 0.95f, 10, 57);
            break;
        case V5:
            sImpl = std::make_unique<SileroV5>();
            setThreshold(0.7f, 0.7f, 0.5f, 0.95f, 10, 57);
            break;
        default:
            throw std::invalid_argument("Invalid SILERO_VER specified");
    }

    sImpl->init_onnx_model(model_path);
}

