#include <vector>

#pragma once

class FeedbackComb {
  public:
    FeedbackComb();
    FeedbackComb(float delay_time, float decay_time, float mul,
                 float samplingRate);
    int setup(float frequency, float decay_time, float mul, float samplingRate);
    float process(float input);
    unsigned int size() { return buf.size(); }
    float val(int i) { return buf.at(i); }
    float getDelay() { return _delay_time; }
    void setDelay(float delay_time);
    float getDecay() { return _decay_time; }
    void setDecay(float decay_time);

  private:
    float _delay_time, _decay_time, _mul, _samplingRate;
    float _alpha;
    int _K, _i, _j;
    std::vector<float> buf;
};
