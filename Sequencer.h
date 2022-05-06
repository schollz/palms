#include <cstdlib>
#include <math.h>
#include <vector>

#pragma once

class Sequencer {
  public:
    Sequencer(){};
    ~Sequencer(){};
    Sequencer(std::vector<float> durs, std::vector<float> vals) {
        _durs = durs;
        _vals = vals;
    };
    bool tick() {
        _dur += 1.0;
        if (_dur < _durs.at(_i)) {
            return false;
        }
        _dur = 0;
        _i += 1.0;
        if (_i >= _durs.size()) {
            _i = 0;
        }
        return true;
    };
    float val() { return _vals.at(_i); };

  private:
    float _i;
    float _dur;
    std::vector<float> _durs;
    std::vector<float> _vals;
};
