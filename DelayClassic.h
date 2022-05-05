#include "CombFeedback.h"
#include "Delay.h"
#include "math.h"
#pragma once

class DelayClassic {
  public:
    DelayClassic(){};
    DelayClassic(float max_delay_time, float delay_time, float decay_time,
                 float max_offset, float offset, float fs) {
        setup(max_delay_time, delay_time, decay_time, max_offset, offset, fs);
    }
    ~DelayClassic(){};

    void setup(float max_delay_time, float delay_time, float decay_time,
               float max_offset, float offset, float fs);

    float process();
    void process_block(unsigned int n);
    void setDelay(float delay_time){_delay_time = delay_time};
    void setDecay(float decay_time){_decay_time = decay_time};
    void setOffset(float offset){_offset = offset};

  private:
    int _i = 0;
    static const float half_pi = M_PI / 2.0;
    float _interp = 0;     // > 0 = interpolating, >1 = done
    float _interp_inc = 0; // samples
    unsigned int _delay_time, _decay_time, _offset, _max_delay_time;
    CombFeedback _comb[2];
    Delay _delay[2];
};
