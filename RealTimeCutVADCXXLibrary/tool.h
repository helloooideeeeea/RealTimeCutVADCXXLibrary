//
// Created by Yasushi Sakita on 25/02/01.
//

#ifndef REALTIMECUTVAD_COMMON_H
#define REALTIMECUTVAD_COMMON_H

#include <vector>

std::vector<uint8_t> framesToWavData(const std::vector<float_t>& audio_frames, int sampleRate);


#endif //REALTIMECUTVAD_COMMON_H
