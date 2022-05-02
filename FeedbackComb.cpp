#include "FeedbackComb.h"
#include <math.h>
#include <stdexcept>

FeedbackComb::FeedbackComb() {}

FeedbackComb::FeedbackComb(float delay, float decay, float mul,
                           float samplingRate) {
    int ret;
    if ((ret = setup(delay, decay, mul, samplingRate)))
        throw std::runtime_error("feedback comb: decay is less then delay");
}

int FeedbackComb::setup(float delay, float decay, float mul,
                        float samplingRate) {
    if (decay < delay) {
        return -1;
    }
    _mul = mul;
    _samplingRate = samplingRate;
    _alpha = pow(0.001, delay / decay);
    _delay = delay;
    _K = static_cast<int>(round(_samplingRate * delay));
    buf.resize(static_cast<int>(round(_samplingRate * delay + 2)));
    // initialize to 0
    for (unsigned i = 0; i < buf.size(); i++)
        buf.at(i) = 0;

    _i = buf.size();
    _j = buf.size() - _K;
    return 0;
}

float FeedbackComb::process(float input) {
    // store the current vlaue
    _i += 1;
    if (_i >= buf.size()) {
        _i = 0;
    }
    buf.at(_i) = input + (_alpha * buf.at(_i));

    // get the previous value
    _j += 1;
    if (_j >= buf.size()) {
        _j = 0;
    }
    return buf.at(_j) * _mul;
}
