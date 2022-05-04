#include "CombFeedback.h"
#include "Delay.h"
#include "math.h"
#pragma once

class StereoDelay {
  public:
    StereoDelay(){};
    StereoDelay(float delay_time[2], float decay_time[2], float offset[2],
                float fs) {
        setup(delay_time, decay_time, offset, fs);
    }
    ~StereoDelay(){};

    void setup(float delay_time[2], float decay_time[2], float offset[2],
               float fs);

    void process();
    void setDelay(unsigned int channel, float delay_time);

  private:
    bool _interpolating;
    int _interpolationTime = 0;
    const unsigned int _interpolationTimeMax = 1000; // samples
    unsigned int _i[2];
    CombFeedback _comb[4];
    Delay _delay[4];
};
