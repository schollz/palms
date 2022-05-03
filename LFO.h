#pragma once

class LFO {
  public:
    typedef enum {
        sine,
        triangle,
        square,
        sawtooth,
        numOscTypes,
    } Type;

    LFO(){};
    LFO(float fs, float frequency, LFO::Type type = sine) {
        setup(fs, frequency, type);
    }
    ~LFO(){};

    void setup(float fs, float frequency, LFO::Type type = sine);

    void process(unsigned int n);
    void setType(LFO::Type type) { type_ = type; }
    void setFrequency(float frequency) { frequency_ = frequency; }
    void setPhase(float phase) { phase_ = phase; }
    float val() { return _out; }

    float getPhase() { return phase_; }
    float getFrequency() { return frequency_; }
    int getType() { return type_; }

  private:
    float out_;
    float phase_;
    float frequency_;
    float invSampleRate_;
    unsigned int type_ = sine;
    void computePhase();
};
