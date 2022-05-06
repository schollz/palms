#include "I1P.h"
#include "MonoFilePlayer.h"
#include "SawVoice.h"
#include "Sequencer.h"
#include <Bela.h>
#include <chrono>
#include <cmath>
#include <vector>

static const int NUM_VOICES = 6;

std::string gFilename = "drums.wav";
MonoFilePlayer gPlayer;

SawVoice voice[NUM_VOICES];
I1P* dcBlock[2];

float beats;
float bpm = 57;

// timing measurements
bool timedRender = false;
float timingResult = 0.0;
std::chrono::steady_clock::time_point timingStart;
std::chrono::steady_clock::time_point timingEnd;

// sequencer
Sequencer sequence[NUM_VOICES];

bool setup(BelaContext* context, void* userData) {

    // setup sequencer
    std::vector<float> beats = {3.0, 3.0, 6.0, 4.0};
    sequence[0] = Sequencer(std::vector<float>{3, 3, 6, 4},
                            std::vector<float>{28, 28, 29, 26});
    sequence[1] = Sequencer(beats, std::vector<float>{43, 48, 45, 50});
    sequence[2] = Sequencer(beats, std::vector<float>{59, 57, 57, 59});
    sequence[3] = Sequencer(beats, std::vector<float>{64, 69, 65, 59});
    sequence[4] = Sequencer(beats, std::vector<float>{59, 52, 48, 55});
    sequence[5] = Sequencer(beats, std::vector<float>{79, 72, 72, 71});

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
        for (unsigned int i = 0; i < NUM_VOICES; i++) {
            if (sequence[i].tick() == true) {
                rt_printf("%d[%2.1f] ", i, sequence[i].val());
                voice[i].setNote(sequence[i].val());
            }
        }
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
