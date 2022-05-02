#include "Allpass.h"
#include "FeedbackComb.h"
#include <vector>

#pragma once

#define ZVERB_COMB 10
#define ZVERB_ALLPASS 4

class ZVerb {
  public:
    ZVerb();
    ZVerb(float wet, float roomsize, float samplingRate);
    int setup(float wet, float roomsize, float samplingRate);
    float process(float input);

  private:
    float wet_, roomsize_, samplingRate_;
    FeedbackComb comb[ZVERB_COMB];
    Allpass allpass[ZVERB_ALLPASS];
};
