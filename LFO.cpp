#include "LFO.h"
#include <cmath>
#include <libraries/math_neon/math_neon.h>

void LFO::setup(float fs, float frequency, LFO::Type type) {
    frequency_ = frequency;
    invSampleRate_ = 1.0 / fs;
    setType(type);
    phase_ = M_PI;
}

float LFO::process(unsigned int n) {
    for (unsigned int i = 0; i < n; i++) {
        if (phase_ >= M_PI)
            phase_ -= 2.0f * (float)M_PI;
        phase_ += 2.0f * (float)M_PI * frequency_ * invSampleRate_;
    }
    switch (type_) {
    default:
    case sine:
        out_ = sinf_neon(phase_);
        break;
    case triangle:
        if (phase_ > 0) {
            out_ = -1 + (2 * phase_ / (float)M_PI);
        } else {
            out_ = -1 - (2 * phase_ / (float)M_PI);
        }
        break;
    case square:
        if (phase_ > 0) {
            out_ = 1;
        } else {
            out_ = -1;
        }
        break;
    case sawtooth:
        out_ = 1.f / (float)M_PI * phase_;
        break;
    }
}
