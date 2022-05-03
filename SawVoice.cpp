#include "SawVoice.h"
#include "Utils.h"
#include <Bela.h>
#include <libraries/Biquad/Biquad.h>
#include <math.h>
#include <stdexcept>
#include <time.h>

float sdrandfloat(float a, float b) {
    return ((b - a) * ((float)rand() / RAND_MAX)) + a;
}

SawVoice::SawVoice() {}

SawVoice::SawVoice(float frequency, float samplingRate) {
    int ret;
    if ((ret = setup(frequency, samplingRate)))
        throw std::runtime_error("OnePole: cutoff is above Nyquist");
}

int SawVoice::setup(float frequency, float samplingRate) {
    if (frequency < 1) {
        return -1;
    }
    _samplingRate = samplingRate;
    _frequency = frequency / 2;
    _brightness = 0.6;
    _detuning = sdrandfloat(0.002, 0.003);
    _detuningL = sdrandfloat(0, _detuning);
    _detuningR = sdrandfloat(0, _detuning);
    _pan = 0.5;
    _amp = 0.1;

    Biquad::Settings settings{
        .fs = _samplingRate,
        .cutoff = _frequency,
        .type = Biquad::lowpass,
        .q = 0.707,
        .peakGainDb = 0,
    };
    lpFilter.setup(settings);

    for (unsigned int i = 0; i < 2; i++) {
        osc[i].setup(_samplingRate);
        osc[i].setFrequency(_frequency);
        osc[i].setAmp(_amp);
    }

    lfo[0] = LFO(sdrandfloat(1.0 / 30.0, 1.0 / 10.0), _samplingRate);
    lfo[1] = LFO(sdrandfloat(1.0 / 60.0, 1.0 / 10.0), _samplingRate);
    update();
    return 0;
}

void SawVoice::update() {
    float fc = _frequency * pow(100, _brightness);
    if (fc > 18000) {
        fc = 18000;
    }
    lpFilter.setFc(fc);
}

void SawVoice::setAmp(float amp) { _amp = amp; }

void SawVoice::setFrequency(float frequency) {
    _frequency = frequency;
    update();
}

void SawVoice::setBrightness(float brightness) {
    _brightness = brightness;
    update();
}

void SawVoice::setDetuning(float detuning) {
    _detuning = detuning;
    _detuningL = sdrandfloat(0, _detuning);
    _detuningR = sdrandfloat(0, _detuning);
}

void SawVoice::setPan(float pan) { _pan = pan; }

void SawVoice::process_block(unsigned int n) {
    for (unsigned int i = 0; i < NUM_LFOS; i++) {
        lfo[i].process(n);
    }
    lpFilter.setFc(utils::linexp(lfo[0], -1, 1, frequency_ / 2, 12000));
}

float SawVoice::process(unsigned int channel) {
    if (channel > 1) {
        return 0;
    }
    float out = 0;
    for (unsigned int i = 0; i < NUM_OSC; i++) {
        if (i == 0) {
            out += osc[i].process(_frequency * (1 + _detuning + _detuningL));
        } else {
            out += osc[i].process(_frequency * (1 - _detuning - _detuningR));
        }
    }
    out = lpFilter.process(out) * _amp;
    _pan = utils::linlin(lfo[1].val(), -1.0, 1.0, 0.25, 0.7);
    if (channel == 0) {
        out *= _pan;
    } else {
        out *= (1 - _pan);
    }
    return out;
}
