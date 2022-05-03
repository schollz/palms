#include "LFO.h"
#include "SawDetuned.h"
#include <libraries/Biquad/Biquad.h>
#include <vector>

#pragma once
#define NUM_LFOS 1
#define NUM_OSC 5

class SawVoice {
  public:
    SawVoice();
    SawVoice(float frequency, float samplingRate);
    int setup(float frequency, float samplingRate);
    void setFrequency(float frequency);
    void setDetuning(float detuning);
    void setAmp(float amp);
    float process(unsigned int channel);
    void process_block(unsigned int n);

  private:
    float _samplingRate;
    float _frequency;
    float _detuning;
    float _pan;
    float _amp;
    Biquad lpFilter;
    SawDetuned osc[NUM_OSC];
    LFO lfo[NUM_LFOS];
    void update();
};
