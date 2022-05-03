#include "LFO.h"
#include "Saw.h"
#include <libraries/Biquad/Biquad.h>
#include <vector>

#pragma once
#define NUM_LFOS 2
#define NUM_OSC 2

class SawDetuned {
  public:
    SawDetuned();
    SawDetuned(float frequency, float samplingRate);
    int setup(float frequency, float samplingRate);
    void setFrequency(float frequency);
    void setBrightness(float brightness);
    void setDetuning(float detuning);
    void setPan(float pan);
    void setAmp(float amp);
    float process(unsigned int channel);
    void process_block(unsigned int n);

  private:
    float _samplingRate;
    float _frequency;
    float _brightness;
    float _detuning, _detuningL, _detuningR;
    float _pan;
    float _amp;
    Biquad lpFilter;
    Saw osc[NUM_OSC];
    LFO lfo[NUM_LFOS];
    void update();
};
