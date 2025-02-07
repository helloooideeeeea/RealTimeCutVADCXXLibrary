//
// Created by Yasushi Sakita on 25/02/01.
//

#ifndef REALTIMECUTVAD_PROCESS_H
#define REALTIMECUTVAD_PROCESS_H

#include "realtime_cut_vad.h"
#include <webrtc_audio_processing/webrtc-audio-processing-1/modules/audio_processing/include/audio_processing.h>


struct RealTimeCutVAD::ProcessImpl {
    webrtc::AudioProcessing* apm;
    webrtc::StreamConfig inputConfig;
    webrtc::StreamConfig outputConfig;
    std::vector<float_t> remaining_audio_data_1;
    std::vector<float_t> remaining_audio_data_2;
    int input_audio_split_chunk_size;
    int apm_sample_size;
    int silero_vad_sample_size = 512;
    std::vector<std::vector<float_t>> split_audio_data_with_remainder(std::vector<float_t>& remaining_audio_data, std::vector<float_t>& input_audio_data, size_t chunk_size);
};

#endif //REALTIMECUTVAD_PROCESS_H
