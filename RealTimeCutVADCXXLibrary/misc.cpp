//
// Created by Yasushi Sakita on 25/02/01.
//

#include "misc.h"

int getSampleRateValue(SAMPLE_RATE choice_sample_rate) {
    switch (choice_sample_rate) {
        case K_8:
            return K_8_SAMPLE_RATE;
        case K_16:
            return K_16_SAMPLE_RATE;
        case K_24:
            return K_24_SAMPLE_RATE;
        case K_48:
            return K_48_SAMPLE_RATE;
        default:
            throw std::invalid_argument("Invalid SAMPLE_RATE specified");
    }
}