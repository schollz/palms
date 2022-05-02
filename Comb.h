#include <vector>

#pragma once 

class Comb {
public: 
	Comb();
	Comb(float seconds, float decaytime, float mul, float samplingRate);
	int setup(float frequency, float decaytime, float mul, float samplingRate);
	float process(float input);
	unsigned int size() { return buf.size(); }
	float val(int i) { return buf.at(i); }

private:
	float _seconds,_mul,_samplingRate;
	float _alpha;
	int _K,_i,_j;
	std::vector<float> buf;
};
