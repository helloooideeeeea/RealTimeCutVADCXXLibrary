//
//  c_wrapper.cpp
//  RealTimeCutVADCXXLibrary
//
//  Created by Yasushi Sakita on 2025/02/06.
//

#include "c_wrapper.h"
#include "realtime_cut_vad.h"


VADInstanceHandle create_vad_instance() {
    return new RealTimeCutVAD();
}

void destroy_vad_instance(VADInstanceHandle instance) {
    delete static_cast<RealTimeCutVAD*>(instance);
}

void set_vad_callback(VADInstanceHandle instance, void* context,
                      VoiceStartCallback start_cb, VoiceEndCallback end_cb) {
    static_cast<RealTimeCutVAD*>(instance)->setCallback(context, start_cb, end_cb);
}

void set_vad_sample_rate(VADInstanceHandle instance, int sample_rate) {
    static_cast<RealTimeCutVAD*>(instance)->setSampleRate(static_cast<SAMPLE_RATE>(sample_rate));
}

void set_vad_threshold(VADInstanceHandle instance, float vad_start_prob,
                       float vad_end_prob, float start_true_ratio,
                       float end_false_ratio, int start_frame_count, int end_frame_count) {
    static_cast<RealTimeCutVAD*>(instance)->setThreshold(vad_start_prob, vad_end_prob,
                                                         start_true_ratio, end_false_ratio,
                                                         start_frame_count, end_frame_count);
}

void set_vad_model(VADInstanceHandle instance, int model_version, const char* model_path) {
    static_cast<RealTimeCutVAD*>(instance)->setModel(static_cast<SILERO_VER>(model_version), std::string(model_path));
}

void process_vad_audio(VADInstanceHandle instance, const float* audio_data, size_t length) {
    std::vector<float_t> audio(audio_data, audio_data + length);
    static_cast<RealTimeCutVAD*>(instance)->process(audio);
}
