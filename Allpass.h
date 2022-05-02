#include <vector>

#pragma once

class Allpass {
  public:
    Allpass();
    Allpass(float delay, float decay, float mul, float samplingRate);
    int setup(float frequency, float decay, float mul, float samplingRate);
    float process(float input);
    unsigned int size() { return _s.size(); }
    float val(int i) { return _s.at(i); }

  private:
    float _delay, _mul, _samplingRate, _k;
    int _D, _t, _tD;
    std::vector<float> _s;
};
