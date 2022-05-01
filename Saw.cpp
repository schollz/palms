#include "Saw.h"
#include <Bela.h>
#include <math.h>
#include <stdexcept>
#include <libraries/Oscillator/Oscillator.h>
#include <libraries/Biquad/Biquad.h>
#include <libraries/Oscillator/Oscillator.h>
#include <libraries/Biquad/Biquad.h>
#include <libraries/ADSR/ADSR.h>

#define NUM_OSC 2 // total number of oscillators


Saw::Saw() {}

Saw::Saw(float frequency, float samplingRate) {
	int ret;
	if((ret = setup(frequency, samplingRate)))
		throw std::runtime_error("OnePole: cutoff is above Nyquist");
}

int Saw::setup(float frequency, float samplingRate) {
	if (frequency < 1) {
		return -1;
	}
	_samplingRate=samplingRate;
	_frequency=frequency;
	_brightness=0.1;
	_detuning=0.001;
	_pan=0.5;
	_amp=0.1;
	_attack=2;
	_decay=0.25;
	_sustain=0.9;
	_release=5.0;
	_envAmp=0;

	envelope.setAttackRate(_attack * _samplingRate);
	envelope.setDecayRate(_decay * _samplingRate);
	envelope.setReleaseRate(_release * _samplingRate);
	envelope.setSustainLevel(_sustain);

	Biquad::Settings settings{
		.fs = _samplingRate,
		.cutoff = _frequency,
		.type = Biquad::lowpass,
		.q = 0.707,
		.peakGainDb = 0,
	};
	for (unsigned int i=0;i<2;i++) {
		lpFilter[i].setup(settings);
	}

	for (unsigned int i=0;i<NUM_OSC;i++) {
		osc[i].setup(_samplingRate,Oscillator::sawtooth);
	}
	return 0;
}

void Saw::update() {
	for (unsigned int i=0;i<2;i++) {
		lpFilter[i].setFc(_frequency*pow(10,_brightness));
	}
}

void Saw::setAmp(float amp) {
	_amp=amp;
}

void Saw::setFrequency(float frequency) {
	_frequency=frequency;
	update();
}

void Saw::setBrightness(float brightness) {
	_brightness=brightness;
	update();
}

void Saw::setDetuning(float detuning) {
	_detuning=detuning;
}

void Saw::setPan(float pan) {
	_pan=pan;
}

void Saw::gate(bool on) {
	envelope.gate(on);
}

float Saw::process(unsigned int channel) {
	if (channel>1) {
		return 0;
	}
	float amp=_pan;
	if (channel==1) {
		amp=1-_pan;
	} else {
		// process envelope every two times
		_envAmp = envelope.process();
	}
	amp=amp*_amp/2*_envAmp;
	// mix the two oscillators
	float out=0;
	for (unsigned int i=0;i<NUM_OSC;i++) {
		out += osc[i].process(
			_frequency * 
			(1 + _detuning*(i*2-1)) 
		);
	} 
	out = lpFilter[channel].process(out * amp);
	return out;
}