#include <vector>

#pragma once

class Allpass {
  public:
    Allpass();
    Allpass(float delay, float decay, float mul, float samplingRate);
    int setup(float frequency, float decay, float mul, float samplingRate);
    float process(float input);
    unsigned int size() { return buf.size(); }
    float val(int i) { return buf.at(i); }

  private:
    float _delay, _mul, _samplingRate;
    int _k, _t, _tD;
    std::vector<float> _s;
};
