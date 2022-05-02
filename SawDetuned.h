#include <libraries/ADSR/ADSR.h>
#include <libraries/Biquad/Biquad.h>
#include <libraries/Oscillator/Oscillator.h>

#pragma once

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
    void gate(bool on);
    void toggle();
    float process(unsigned int channel);

  private:
    float _samplingRate;
    float _frequency;
    float _brightness;
    float _detuning;
    float _pan;
    float _amp, _envAmp;
    float _attack, _decay, _sustain, _release;
    bool _gate;
    ADSR envelope;
    Biquad lpFilter[2];
    Oscillator osc[2];
    void update();
};
