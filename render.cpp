#include "I1P.h"
#include "MonoFilePlayer.h"
#include "SawVoice.h"
#include "Sequencer.h"
#include "flange.h"
#include "reverb.h"
#include <Bela.h>
#include <chrono>
#include <cmath>
#include <libraries/ADSR/ADSR.h>
#include <libraries/Biquad/Biquad.h>
#include <vector>

// profiling
BelaCpuData gCpuRender = {
    .count = 100,
};

static const int NUM_VOICES = 6;

std::string gFilename = "drums.wav";
MonoFilePlayer gPlayer;

SawVoice voice[NUM_VOICES];
I1P* dcBlock[2];

float beats;
float bpm = 50;

// timing measurements
bool timedRender = false;
float timingResult = 0.0;
std::chrono::steady_clock::time_point timingStart;
std::chrono::steady_clock::time_point timingEnd;

// sequencer
Sequencer sequence[6];

Biquad lowshelf, hishelf, bassboost;

ADSR envelope;

// Processing buffer passed from each effect to the next.
float *ch0, *ch1, *gMaster_Envelope;
int gNframes = 0;

// Reverb

Reverb zita[2];
tflanger* delayline[2];
tflanger* chorus[2];

static void loop(void*) {
    while (!Bela_stopRequested()) {
        BelaCpuData* data = Bela_cpuMonitoringGet();
        printf("total: %.2f%%, render: %.2f%%\n", data->percentage,
               gCpuRender.percentage);
        usleep(1000000);
    }
}

bool setup(BelaContext* context, void* userData) {
    srand(time(NULL));

    // enable CPU monitoring for the whole audio thread
    Bela_cpuMonitoringInit(100);
    Bela_runAuxiliaryTask(loop);

    // reverb

    gNframes = context->audioFrames;
    ch0 = (float*)malloc(sizeof(float) * context->audioFrames);
    ch1 = (float*)malloc(sizeof(float) * context->audioFrames);
    gMaster_Envelope = (float*)malloc(sizeof(float) * context->audioFrames);

    // setup sequencer
    std::vector<float> beats = {3.0, 5.0, 4.0, 6.0};
    sequence[0] = Sequencer(beats, std::vector<float>{28, 28, 29, 26});
    sequence[1] = Sequencer(beats, std::vector<float>{43, 48, 45, 50});
    sequence[2] = Sequencer(beats, std::vector<float>{59, 57, 57, 59 - 12});
    sequence[3] = Sequencer(beats, std::vector<float>{64, 69, 65, 59});
    sequence[4] = Sequencer(beats, std::vector<float>{59, 52, 48, 55});
    sequence[5] = Sequencer(beats, std::vector<float>{79, 72, 72, 71 - 12});

    // Set ADSR parameters
    envelope.setAttackRate(11 * context->audioSampleRate);
    envelope.setDecayRate(1 * context->audioSampleRate);
    envelope.setReleaseRate(1.0 * context->audioSampleRate);
    envelope.setSustainLevel(1.0);
    envelope.gate(true);

    Biquad::Settings settings{
        .fs = context->audioSampleRate,
        .cutoff = 150,
        .type = Biquad::lowshelf,
        .q = 0.707,
        .peakGainDb = 8,
    };
    lowshelf.setup(settings);
    Biquad::Settings settings2{
        .fs = context->audioSampleRate,
        .cutoff = 6050,
        .type = Biquad::highshelf,
        .q = 0.707,
        .peakGainDb = -8,
    };
    hishelf.setup(settings2);
    Biquad::Settings settings3{
        .fs = context->audioSampleRate,
        .cutoff = 60,
        .type = Biquad::peak,
        .q = 0.707,
        .peakGainDb = 8,
    };
    bassboost.setup(settings3);

    if (!gPlayer.setup(gFilename)) {
        rt_printf("Error loading audio file '%s'\n", gFilename.c_str());
        return false;
    }

    // Print some useful info
    rt_printf("Loaded the audio file '%s' with %d frames (%.1f seconds)\n",
              gFilename.c_str(), gPlayer.size(),
              gPlayer.size() / context->audioSampleRate);

    for (unsigned int i = 0; i < NUM_VOICES; i++) {
        float offset = (float)rand() / RAND_MAX * 0.5 - 0.25; // offset in cents
        voice[i] = SawVoice(98.1 * (i + 1), offset, context->audioSampleRate);
        voice[i].setDetuning((float)rand() / RAND_MAX *
                             1.0); // detuning in cents
    }

    for (unsigned int i = 0; i < 2; i++) {
        dcBlock[i] = new I1P(10.0 / context->audioSampleRate);
        delayline[i] =
            tflanger_init(delayline[i], 5.0, context->audioSampleRate);
        chorus[i] = tflanger_init(delayline[i], 0.2, context->audioSampleRate);
        zita[i].init(context->audioSampleRate, false,
                     context->audioFrames); // no ambisonic processing
        zita[i].set_opmix(0.6);
        zita[i].set_xover(1200.0);
        zita[i].set_rtlow(4.0 + (float)rand() / RAND_MAX * 4);
        zita[i].set_rtmid(6.0 + (float)rand() / RAND_MAX * 2);
        zita[i].set_delay(3.5 + (float)rand() / RAND_MAX * 1.03);
        zita[i].set_fdamp(6000 + (float)rand() / RAND_MAX * 1000);
        tflanger_setPreset(delayline[i], 1);
        tflanger_setPreset(chorus[i], 0);

        tflanger_setFinalGain(delayline[i], 0.0);
        tflanger_setFinalGain(chorus[i], 0.0);
        tflanger_setLfoWidth(chorus[i], 0.0 + (float)rand() / RAND_MAX * 0.005);
        tflanger_setLfoDepth(chorus[i],
                             0.03 + (float)rand() / RAND_MAX * 0.005);
    }

    return true;
}

void render(BelaContext* context, void* userData) {
    float beats_old = beats;
    beats += (bpm / 60) * context->audioFrames / context->audioSampleRate;
    if (floor(beats) != floor(beats_old)) {
        // randomly get
        // if ((float)rand() / RAND_MAX < 0.05) {
        //    envelope.gate(false);
        //} else {
        //    envelope.gate(true);
        // }
        // new beat
        rt_printf("beat (%2.0f bpm): %2.3f ", bpm, beats);
        rt_printf("amp%2.3f", voice[0].getAmp());
        for (unsigned int i = 0; i < NUM_VOICES; i++) {
            if (sequence[i].tick() == true) {
                rt_printf("%d[%2.1f] ", i, sequence[i].val());
                voice[i].setNote(sequence[i].val());
            }
        }
        rt_printf("\n");
    }
    Bela_cpuTic(&gCpuRender);

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
            out = lowshelf.process(out / 2.0);
            out = hishelf.process(out);
            out = bassboost.process(out);
            out -= dcBlock[channel]->process(out);
            out = tanh(out * 0.15);
            gMaster_Envelope[n] = envelope.process();
            // audioWrite(context, n, channel, out);
            out *= gMaster_Envelope[n];
            if (channel == 0) {
                ch0[n] = out;
            } else {
                ch1[n] = out;
            }
        }
    }
    tflanger_tick(delayline[0], gNframes, ch0, gMaster_Envelope);
    // tflanger_tick(chorus[0], gNframes, ch0, gMaster_Envelope);
    tflanger_tick(delayline[1], gNframes, ch1, gMaster_Envelope);
    // tflanger_tick(chorus[1], gNframes, ch1, gMaster_Envelope);
    zita[0].tick_mono(gNframes, ch0);
    zita[1].tick_mono(gNframes, ch1);

    for (unsigned int n = 0; n < context->audioFrames; n++) {
        audioWrite(context, n, 0, ch0[n]);
        audioWrite(context, n, 1, ch1[n]);
    }

    // perform the timing on the first render
    Bela_cpuToc(&gCpuRender);
}

void cleanup(BelaContext* context, void* userData) {}
