#include "SawDetuned.h"
#include <Bela.h>
#include <libraries/ADSR/ADSR.h>
#include <libraries/Biquad/Biquad.h>
#include <libraries/Oscillator/Oscillator.h>
#include <math.h>
#include <stdexcept>

#define NUM_OSC 2 // total number of oscillators

SawDetuned::SawDetuned() {}

SawDetuned::SawDetuned(float frequency, float samplingRate) {
    int ret;
    if ((ret = setup(frequency, samplingRate)))
        throw std::runtime_error("OnePole: cutoff is above Nyquist");
}

int SawDetuned::setup(float frequency, float samplingRate) {
    if (frequency < 1) {
        return -1;
    }
    _samplingRate = samplingRate;
    _frequency = frequency;
    _brightness = 0.2;
    _detuning = 0.001;
    _pan = 0.5;
    _amp = 0.5;
    _attack = 0.05;
    _decay = 0.1;
    _sustain = 0.9;
    _release = 0.1;
    _envAmp = 0;
    _gate = false;

    envelope.setAttackRate(_attack * _samplingRate);
    envelope.setDecayRate(_decay * _samplingRate);
    envelope.setReleaseRate(_release * _samplingRate);
    envelope.setSustainLevel(_sustain);

    Biquad::Settings settings{
        .fs = _samplingRate,
        .cutoff = _frequency,
        .type = Biquad::lowpass,
        .q = 0.707,
        .peakGainDb = 0,
    };
    for (unsigned int i = 0; i < 2; i++) {
        lpFilter[i].setup(settings);
    }

    for (unsigned int i = 0; i < NUM_OSC; i++) {
        osc[i].setup(_samplingRate, Oscillator::sawtooth);
    }

    update();
    return 0;
}

void SawDetuned::update() {
    for (unsigned int i = 0; i < 2; i++) {
        lpFilter[i].setFc(_frequency * pow(10, _brightness));
    }
}

void SawDetuned::setAmp(float amp) { _amp = amp; }

void SawDetuned::setFrequency(float frequency) {
    _frequency = frequency;
    update();
}

void SawDetuned::setBrightness(float brightness) {
    _brightness = brightness;
    update();
}

void SawDetuned::setDetuning(float detuning) { _detuning = detuning; }

void SawDetuned::setPan(float pan) { _pan = pan; }

void SawDetuned::gate(bool on) {
    _gate = on;
    envelope.gate(on);
}

void SawDetuned::toggle() {
    _gate = !_gate;
    envelope.gate(_gate);
}

float SawDetuned::process(unsigned int channel) {
    if (channel > 1) {
        return 0;
    }
    float amp = _pan;
    if (channel > 0) {
        amp = 1 - _pan;
    } else {
        // process envelope every two times
        // _envAmp = envelope.process();
    }
    amp = amp * _amp / 10 * envelope.process();
    // mix the two oscillators
    float out = 0;
    for (unsigned int i = 0; i < NUM_OSC; i++) {
        if (i == 0) {
            out += osc[i].process(_frequency * (1 + _detuning));
        } else {
            out += osc[i].process(_frequency * (1 - _detuning));
        }
    }
    out = lpFilter[channel].process(out * amp);
    return out;
}
