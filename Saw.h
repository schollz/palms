#include <libraries/Oscillator/Oscillator.h>
#include <libraries/Biquad/Biquad.h>
#include <libraries/ADSR/ADSR.h>

#pragma once 

class Saw {
public: 
	Saw();
	Saw(float frequency, float samplingRate);
	int setup(float frequency, float samplingRate);
	void setFrequency(float frequency);
	void setBrightness(float brightness);
	void setDetuning(float detuning);
	void setPan(float pan);
	void setAmp(float amp);
	void gate(bool on);
	float process(unsigned int channel);
private:
	float _samplingRate;
	float _frequency;
	float _brightness;
	float _detuning;
	float _pan;
	float _amp, _envAmp;
	float _attack,_decay,_sustain,_release;
	ADSR envelope;
	Biquad lpFilter[2];
	Oscillator osc[2];
	void update();
};
