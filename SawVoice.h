#include "LFO.h"
#include "SawDetuned.h"
#include <libraries/Biquad/Biquad.h>
#include <vector>

#pragma once

class SawVoice {
  public:
    SawVoice();
    SawVoice(float frequency, float samplingRate);
    int setup(float frequency, float samplingRate);
    void setFrequency(float frequency);
    void setNote(float note);
    void setDetuning(float detuning);
    void setAmp(float amp);
    float process(unsigned int channel);
    void process_block(unsigned int n);

  private:
    static const int NUM_LFOS = 1;
    static const int NUM_OSC = 2;
    float _samplingRate;
    float _frequency;
    float _detuning;
    float _amp;
    Biquad lpFilter;
    SawDetuned osc[NUM_OSC];
    LFO lfo[NUM_LFOS];
    void update();
};
