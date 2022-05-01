#include <Bela.h>
#include <cmath>
#include <libraries/Trill/Trill.h>
#include "Saw.h"

// Trill stuff
#define NUM_TOUCH 5 // Number of touches on Trill sensor
Trill touchSensor;
float gTouchLocation[NUM_TOUCH] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
float gTouchSize[NUM_TOUCH] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
int gNumActiveTouches = 0;
unsigned int gTaskSleepTime = 12000; // microseconds
// OnePole freqFilt[NUM_TOUCH], ampFilt[NUM_TOUCH];
float gCutOffFreq = 5, gCutOffAmp = 15;

Saw osc[NUM_TOUCH];


void loop(void*) {
	while(!Bela_stopRequested())
	{
		// Read locations from Trill sensor
		touchSensor.readI2C();
		gNumActiveTouches = touchSensor.getNumTouches();
		for(unsigned int i = 0; i <  gNumActiveTouches; i++) {
			gTouchLocation[i] = touchSensor.touchLocation(i);
			gTouchSize[i] = touchSensor.touchSize(i);
		}
		// For all inactive touches, set location and size to 0
		for(unsigned int i = gNumActiveTouches; i < NUM_TOUCH; i++) {
			gTouchLocation[i] = 0.0;
			gTouchSize[i] = 0.0;
		}
		usleep(gTaskSleepTime);
	}
}

bool setup(BelaContext *context, void *userData)
{
	// Setup a Trill Bar sensor on i2c bus 1, using the default mode and address
	if(touchSensor.setup(1, Trill::BAR) != 0) {
		fprintf(stderr, "Unable to initialise Trill Bar\n");
		return false;
	}
	touchSensor.printDetails();
	// Set and schedule auxiliary task for reading sensor data from the I2C bus
	Bela_runAuxiliaryTask(loop);

	osc[0]=Saw(65.41,context->audioSampleRate);
	osc[1]=Saw(98.1,context->audioSampleRate);
	osc[2]=Saw(82.4,context->audioSampleRate);
	osc[3]=Saw(77.81,context->audioSampleRate);
	for(unsigned int i = 4; i < NUM_TOUCH; i++) {
		osc[i]=Saw(98.0,context->audioSampleRate);
	}

	return true;
}

void render(BelaContext *context, void *userData)
{
	for(unsigned int i = 0; i < NUM_TOUCH; i++) {
		if (gTouchSize[i]>0) {
			osc[i].gate(true);
			osc[i].setPan(gTouchLocation[i]);
		} else {
			osc[i].gate(false);
		}
	}
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) {
			float out=0;
			for (unsigned int i=0;i<NUM_TOUCH;i++) {
				out+=osc[i].process(channel);
			}
			audioWrite(context, n, channel, out);
		}
	}
}

void cleanup(BelaContext *context, void *userData) {}
