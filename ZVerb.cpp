#include "ZVerb.h"
#include <math.h>
#include <stdexcept>

float randfloat(float a, float b) {
    return ((b - a) * ((float)rand() / RAND_MAX)) + a;
}

ZVerb::ZVerb() {}

ZVerb::ZVerb(float wet, float roomsize, float samplingRate) {
    int ret;
    if ((ret = setup(wet, roomsize, samplingRate)))
        throw std::runtime_error("feedback comb: decay is less then delay");
}

int ZVerb::setup(float wet, float roomsize, float samplingRate) {
    if (wet < 0) {
        return -1;
    }
    wet_ = wet;
    roomsize_ = roomsize;
    samplingRate_ = samplingRate;
    for (unsigned int i = 0; i < ZVERB_COMB; i++) {
        comb[i] =
            FeedbackComb(randfloat(0.01, 0.99), randfloat(0.1, 6) * roomsize_,
                         randfloat(0.25, 0.75), samplingRate_);
    }

    for (unsigned int i = 0; i < ZVERB_ALLPASS; i++) {
        allpass[i] = Allpass(randfloat(0.01, 0.99),
                             randfloat(0.1, 6) * roomsize_, 1, samplingRate_);
    }
    return 0;
}

float ZVerb::process(float input) {
    float out = 0;
    for (unsigned int i = 0; i < ZVERB_COMB; i++) {
        out += comb[i].process(input);
    }
    for (unsigned int i = 0; i < ZVERB_ALLPASS; i++) {
        out = allpass[i].process(out);
    }
    return out;
}
