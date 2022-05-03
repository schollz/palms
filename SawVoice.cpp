#include "SawVoice.h"
#include "Utils.h"
#include <Bela.h>
#include <libraries/Biquad/Biquad.h>
#include <math.h>
#include <stdexcept>
#include <time.h>

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
    _detuning = utils::randfloat(0.002, 0.003);
    _amp = 0.1;

    Biquad::Settings settings{
        .fs = _samplingRate,
        .cutoff = _frequency,
        .type = Biquad::lowpass,
        .q = 0.707,
        .peakGainDb = 0,
    };
    lpFilter.setup(settings);

    for (unsigned int i = 0; i < NUM_OSC; i++) {
        osc[i] = SawDetuned(_frequency, _samplingRate);
        osc[i].setDetuning(_detuning);
        osc[i].setAmp(_amp);
    }

    lfo[0] = LFO(utils::randfloat(1.0 / 30.0, 1.0 / 10.0), _samplingRate);
    return 0;
}

void SawVoice::setAmp(float amp) {
    _amp = amp;
    for (unsigned int i = 0; i < NUM_OSC; i++) {
        osc[i].setAmp(_amp);
    }
}

void SawVoice::setNote(float note) { setFrequency(utils::midi_to_freq(note)); }

void SawVoice::setFrequency(float frequency) {
    _frequency = frequency;
    for (unsigned int i = 0; i < NUM_OSC; i++) {
        osc[i].setFrequency(_frequency);
    }
}

void SawVoice::setDetuning(float detuning) {
    _detuning = detuning;
    for (unsigned int i = 0; i < NUM_OSC; i++) {
        osc[i].setDetuning(_detuning);
    }
}

void SawVoice::process_block(unsigned int n) {
    for (unsigned int i = 0; i < NUM_LFOS; i++) {
        lfo[i].process(n);
    }
    for (unsigned int i = 0; i < NUM_OSC; i++) {
        osc[i].process_block(n);
    }
}

float SawVoice::process(unsigned int channel) {
    if (channel > 1) {
        return 0;
    }
    float out = 0;
    for (unsigned int i = 0; i < NUM_OSC; i++) {
        out += osc[i].process(channel);
    }
    // out = lpFilter.process(out) * _amp;
    return out;
}
