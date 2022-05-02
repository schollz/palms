#include "Saw.h"
#include <cmath>

void Saw::setup(float fs) {
    invSampleRate_ = 1.0 / fs;
    phase_ = M_PI;
}

float Saw::process() {
    phase_ += 2.0f * (float)M_PI * frequency_ * invSampleRate_;
    if (phase_ >= M_PI)
        phase_ -= 2.0f * (float)M_PI;

    return 1.f / (float)M_PI * phase_;
}
