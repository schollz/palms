#include <Bela.h>
#include <cmath>
#include <libraries/Trill/Trill.h>
#include <libraries/Scope/Scope.h>
#include "MonoFilePlayer.h"
#include "ZVerb.h"
#include "SawDetuned.h"
#include "I1P.h"

Scope scope;

std::string gFilename = "drums.wav";
MonoFilePlayer gPlayer;

// Trill stuff
#define NUM_TOUCH 5 // Number of touches on Trill sensor
Trill touchSensor;
float gTouchLocation[NUM_TOUCH] = {0.0, 0.0, 0.0, 0.0, 0.0};
float gTouchSize[NUM_TOUCH] = {0.0, 0.0, 0.0, 0.0, 0.0};
float beats = 0;
float bpm = 35;
int gNumActiveTouches = 0;
unsigned int gTaskSleepTime = 12000; // microseconds
// OnePole freqFilt[NUM_TOUCH], ampFilt[NUM_TOUCH];
float gCutOffFreq = 5, gCutOffAmp = 15;

SawDetuned osc[NUM_TOUCH];
ZVerb verb[2];
I1P *dcBlock[2]; 

void loop(void*) {
    return;
    while (!Bela_stopRequested()) {
        // Read locations from Trill sensor
        touchSensor.readI2C();
        gNumActiveTouches = touchSensor.getNumTouches();
        for (unsigned int i = 0; i < gNumActiveTouches; i++) {
            gTouchLocation[i] = touchSensor.touchLocation(i);
            gTouchSize[i] = touchSensor.touchSize(i);
        }
        // For all inactive touches, set location and size to 0
        for (unsigned int i = gNumActiveTouches; i < NUM_TOUCH; i++) {
            gTouchLocation[i] = 0.0;
            gTouchSize[i] = 0.0;
        }
        usleep(gTaskSleepTime);
    }
}

bool setup(BelaContext* context, void* userData) {
	scope.setup(1, context->audioSampleRate);
	
if(!gPlayer.setup(gFilename)) {
    	rt_printf("Error loading audio file '%s'\n", gFilename.c_str());
    	return false;
	}

	// Print some useful info
    rt_printf("Loaded the audio file '%s' with %d frames (%.1f seconds)\n", 
    			gFilename.c_str(), gPlayer.size(),
    			gPlayer.size() / context->audioSampleRate);
	

    // // Setup a Trill Bar sensor on i2c bus 1, using the default mode and
    // address if(touchSensor.setup(1, Trill::BAR) != 0) { 	fprintf(stderr,
    // "Unable to initialise Trill Bar\n"); 	return false;
    // }
    // touchSensor.printDetails();
    // // Set and schedule auxiliary task for reading sensor data from the I2C
    // bus Bela_runAuxiliaryTask(loop);

    osc[0] = SawDetuned(65.41 / 2, context->audioSampleRate);
    osc[1] = SawDetuned(98.1, context->audioSampleRate);
    osc[2] = SawDetuned(82.4, context->audioSampleRate);
    osc[3] = SawDetuned(82.4 * 2, context->audioSampleRate);
    osc[4] = SawDetuned(65.41 * 2, context->audioSampleRate);
	osc[0].setBrightness(0.6);

	// initialize reverb
    for (unsigned int i = 0; i < 2; i++) {
    	verb[i]=ZVerb(0.1,1,context->audioSampleRate);
		dcBlock[i] = new I1P(10.0 / context->audioSampleRate);
    }

    return true;
}

void render(BelaContext* context, void* userData) {
    float beats_old = beats;
    beats += (bpm / 60) * context->audioFrames / context->audioSampleRate;
    if (floor(beats) != floor(beats_old)) {
        // new beat
        rt_printf("beat (%f bpm): %f\n", bpm, beats);
        for (unsigned int i = 0; i < NUM_TOUCH; i++) {
            osc[i].toggle();
        }
    }
    // for(unsigned int i = 0; i < NUM_TOUCH; i++) {
    // 	osc[i].gate(true);
    // 	if (gTouchSize[i]>0) {
    // 		osc[i].gate(true);
    // 		osc[i].setPan(gTouchLocation[i]);
    // 	} else {
    // 		osc[i].gate(false);
    // 	}
    // }
    for (unsigned int n = 0; n < context->audioFrames; n++) {
        // float player = 0.2*gPlayer.process();
        for (unsigned int channel = 0; channel < 2;
             channel++) {
            float out = 0;
            // out += player;
            for (unsigned int i = 0; i < 5; i++) {
                out += osc[i].process(channel);
            }
            out=verb[channel].process(out);
            out -= dcBlock[channel]->process(out);
            audioWrite(context,n,channel,out);
            if (channel==0)
            	scope.log(out);
        }
    }
}

void cleanup(BelaContext* context, void* userData) {}
