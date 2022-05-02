#include "FeedbackComb.h"
#include "Saw.h"
#include <Bela.h>
#include <cmath>
#include <libraries/Trill/Trill.h>

// Trill stuff
#define NUM_TOUCH 5 // Number of touches on Trill sensor
Trill touchSensor;
float gTouchLocation[NUM_TOUCH] = {0.0, 0.0, 0.0, 0.0, 0.0};
float gTouchSize[NUM_TOUCH] = {0.0, 0.0, 0.0, 0.0, 0.0};
float beats = 0;
float bpm = 60;
int gNumActiveTouches = 0;
unsigned int gTaskSleepTime = 12000; // microseconds
// OnePole freqFilt[NUM_TOUCH], ampFilt[NUM_TOUCH];
float gCutOffFreq = 5, gCutOffAmp = 15;

Saw osc[NUM_TOUCH];
FeedbackComb cl[20];
FeedbackComb cr[20];

float randfloat(float a, float b) {
    return ((b - a) * ((float)rand() / RAND_MAX)) + a;
}

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
    // // Setup a Trill Bar sensor on i2c bus 1, using the default mode and
    // address if(touchSensor.setup(1, Trill::BAR) != 0) { 	fprintf(stderr,
    // "Unable to initialise Trill Bar\n"); 	return false;
    // }
    // touchSensor.printDetails();
    // // Set and schedule auxiliary task for reading sensor data from the I2C
    // bus Bela_runAuxiliaryTask(loop);

    osc[0] = Saw(65.41 / 2, context->audioSampleRate);
    osc[1] = Saw(98.1, context->audioSampleRate);
    osc[2] = Saw(82.4, context->audioSampleRate);
    osc[3] = Saw(82.4 * 2, context->audioSampleRate);
    osc[4] = Saw(65.41 * 2, context->audioSampleRate);
    osc[0].setBrightness(0.1);

    for (unsigned int i = 0; i < 20; i++) {
        cl[i] = FeedbackComb(randfloat(0.01, 0.1), randfloat(1, 4),
                             randfloat(0, 0.5), context->audioSampleRate);
        cr[i] = FeedbackComb(randfloat(0.01, 0.1), randfloat(1, 4),
                             randfloat(0, 0.5), context->audioSampleRate);
    }
    // for(unsigned int i = 4; i < NUM_TOUCH; i++) {
    // 	osc[i]=Saw(98.0,context->audioSampleRate);
    // }

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
        for (unsigned int channel = 0; channel < context->audioOutChannels;
             channel++) {
            float out = 0;
            for (unsigned int i = 0; i < NUM_TOUCH; i++) {
                out += osc[i].process(channel);
            }
            if (channel < 1) {
                float out0 = out;
                for (unsigned int j = 0; j < 20; j++) {
                    out += cl[j].process(out0);
                }
                audioWrite(context, n, channel, out);
            } else {
                float out0 = out;
                for (unsigned int j = 0; j < 20; j++) {
                    out += cr[j].process(out0);
                }
                audioWrite(context, n, channel, out0);
            }
        }
    }
}

void cleanup(BelaContext* context, void* userData) {}
