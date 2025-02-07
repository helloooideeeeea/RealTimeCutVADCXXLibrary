//
// Created by Yasushi Sakita on 25/02/01.
//

#ifndef REALTIMECUTVAD_ALGORITHM_H
#define REALTIMECUTVAD_ALGORITHM_H

#include "realtime_cut_vad.h"

struct RealTimeCutVAD::AlgorithmImpl {
    bool recording;
    std::vector<bool> pre_vad_list;
    std::vector<bool> start_vad_list;
    std::vector<float_t> audio_buffer;
    std::vector<float_t> pre_audio_buffer;

    void resetVariables();
    bool trueProbability(const std::vector<bool>& list, float prob);
    bool falseProbability(const std::vector<bool>& list, float prob);
};

#endif //REALTIMECUTVAD_ALGORITHM_H
