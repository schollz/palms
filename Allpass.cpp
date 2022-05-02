// A Schroeder allpass filter is given by the difference equations
// https://doc.sccode.org/Classes/AllpassN.html
// s(t) = x(t) + k * s(t - D)
// y(t) = -k * s(t) + s(t - D)

#include "Allpass.h"
#include <math.h>
#include <stdexcept>

Allpass::Allpass() {}

Allpass::Allpass(float delay, float decay, float mul, float samplingRate) {
    int ret;
    if ((ret = setup(delay, decay, mul, samplingRate)))
        throw std::runtime_error("feedback comb: decay is less then delay");
}

int Allpass::setup(float delay, float decay, float mul, float samplingRate) {
    if (decay < delay) {
        return -1;
    }
    _mul = mul;
    _samplingRate = samplingRate;
    _delay = delay;
    _k = pow(0.001, delay / decay);
    _D = static_cast<int>(round(_samplingRate * delay));
    _s.resize(static_cast<int>(round(_samplingRate * delay + 2)));
    // initialize to 0
    for (unsigned i = 0; i < _s.size(); i++)
        _s.at(i) = 0;

    _t = _s.size();
    _tD = _s.size() - _D;
    return 0;
}

float Allpass::process(float input) {
    _t += 1;
    if (_t >= _s.size()) {
        _t = 0;
    }
    // s(t) = x(t) + k * s(t - D)
    _s.at(_t) = input + (_k * _s.at(tD));

    _tD += 1;
    if (_tD >= _s.size()) {
        _tD = 0;
    }
    // y(t) = -k * s(t) + s(t - D)
    return -1 * _k * _s.at(t) + _s(tD);
}
