#include <vector>

#pragma once

class Delay {
  public:
    Delay();
    Delay(float delay, float samplingRate);
    int setup(float frequency, float samplingRate);
    float process(float input);

  private:
    float _delay, _samplingRate;
    int _K, _i, _j;
    std::vector<float> buf;
};
