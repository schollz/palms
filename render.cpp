#include "I1P.h"
#include "MonoFilePlayer.h"
#include "SawVoice.h"
#include <Bela.h>
#include <chrono>
#include <cmath>

static const int NUM_VOICES = 6;

std::string gFilename = "drums.wav";
MonoFilePlayer gPlayer;

SawVoice voice[NUM_VOICES];
I1P* dcBlock[2];

float beats;
float bpm = 60;

// timing measurements
bool timedRender = false;
float timingResult = 0.0;
std::chrono::steady_clock::time_point timingStart;
std::chrono::steady_clock::time_point timingEnd;

bool setup(BelaContext* context, void* userData) {

    if (!gPlayer.setup(gFilename)) {
        rt_printf("Error loading audio file '%s'\n", gFilename.c_str());
        return false;
    }

    // Print some useful info
    rt_printf("Loaded the audio file '%s' with %d frames (%.1f seconds)\n",
              gFilename.c_str(), gPlayer.size(),
              gPlayer.size() / context->audioSampleRate);

    voice[0] = SawVoice(98.1, context->audioSampleRate);
    for (unsigned int i = 1; i < NUM_VOICES; i++) {
        voice[i] = SawVoice(98.1 * (i + 1), context->audioSampleRate);
    }

    for (unsigned int i = 0; i < 2; i++) {
        dcBlock[i] = new I1P(10.0 / context->audioSampleRate);
    }

    return true;
}

void render(BelaContext* context, void* userData) {
    float beats_old = beats;
    beats += (bpm / 60) * context->audioFrames / context->audioSampleRate;
    if (floor(beats) != floor(beats_old)) {
        // new beat
        rt_printf("beat (%2.0f bpm): %2.3f ", bpm, beats);
        timedRender = true;
    }

    if (timedRender == true) {
        timingStart = std::chrono::steady_clock::now();
    }

    // process block
    for (unsigned int i = 0; i < NUM_VOICES; i++) {
        voice[i].process_block(context->audioFrames);
    }

    for (unsigned int n = 0; n < context->audioFrames; n++) {
        // float player = 0.2 * gPlayer.process();
        for (unsigned int channel = 0; channel < 2; channel++) {
            float out = 0;
            // out += player;
            for (unsigned int i = 0; i < NUM_VOICES; i++) {
                out += voice[i].process(channel);
            }
            out -= dcBlock[channel]->process(out);
            audioWrite(context, n, channel, out);
        }
    }

    // perform the timing on the first render
    if (timedRender == true) {
        timingEnd = std::chrono::steady_clock::now();
        timingResult =
            (std::chrono::duration_cast<std::chrono::nanoseconds>(timingEnd -
                                                                  timingStart)
                 .count() /
             1000.0) /
            (1000000.0 / context->audioSampleRate * context->audioFrames) * 100;
        rt_printf(" (cpu: %2.1f%)\n", timingResult);
        timedRender = false;
    }
}

void cleanup(BelaContext* context, void* userData) {}
