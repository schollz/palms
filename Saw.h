#include <math.h>
#include <cstdlib>

#pragma once

class Saw {
  public:
    Saw(){};
    Saw(float fs) { setup(fs); }
    ~Saw(){};

    void setup(float fs);

    float process();
    void setFrequency(float frequency) { frequency_ = frequency; }
    void setPhase(float phase) { phase_ = phase; }
    void setAmp(float mul) { mul_ = mul; }
    float getPhase() { return phase_; }
    float getFrequency() { return frequency_; }

  private:
    float phase_;
    float frequency_;
    float invSampleRate_;
    float mul_;
};
