#include "Comb.h"
#include <math.h>
#include <stdexcept>

Comb::Comb() {}

Comb::Comb(float seconds, float decaytime, float mul, float samplingRate) {
    int ret;
    if ((ret = setup(seconds, decaytime, mul, samplingRate)))
        throw std::runtime_error("comb: cutoff is above Nyquist");
}

int Comb::setup(float seconds, float decaytime, float mul, float samplingRate) {
    if (samplingRate < 1) {
        return -1;
    }
    _mul = mul;
    _samplingRate = samplingRate;
    _alpha =
        pow(0.001,
            seconds / decaytime); // exp(-6.90775527898/ (decaytime/seconds));
    _seconds = seconds;
    _K = static_cast<int>(round(_samplingRate * seconds));
    buf.resize(static_cast<int>(round(_samplingRate * seconds + 2)));
    // initialize to 0
    for (unsigned i = 0; i < buf.size(); i++)
        buf.at(i) = 0;

    _i = buf.size();
    _j = buf.size() - _K;
    return 0;
}

float Comb::process(float input) {
    // store the current vlaue
    _i += 1;
    if (_i >= buf.size()) {
        _i = 0;
    }
    // https://en.wikipedia.org/wiki/Comb_filter
    // buf.at(_i) = input + (_alpha * buf.at(_i));
    buf.at(_i) = input;

    // get the previous value
    _j += 1;
    if (_j >= buf.size()) {
        _j = 0;
    }
    return (input + _alpha * buf.at(_j)) * _mul;
    // return buf.at(_j) * _mul;
}
