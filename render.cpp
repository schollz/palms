#include "I1P.h"
#include "MonoFilePlayer.h"
#include "SawDetuned.h"
#include "ZVerb.h"
#include <Bela.h>
#include <cmath>
#include <libraries/Scope/Scope.h>

#define NUM_VOICES 5

Scope scope;

std::string gFilename = "drums.wav";
MonoFilePlayer gPlayer;

SawDetuned osc[NUM_VOICES];
ZVerb verb[2];
I1P *dcBlock[2];

float beats;
float bpm = 60;

bool setup(BelaContext *context, void *userData) {
  scope.setup(1, context->audioSampleRate);

  if (!gPlayer.setup(gFilename)) {
    rt_printf("Error loading audio file '%s'\n", gFilename.c_str());
    return false;
  }

  // Print some useful info
  rt_printf("Loaded the audio file '%s' with %d frames (%.1f seconds)\n",
            gFilename.c_str(), gPlayer.size(),
            gPlayer.size() / context->audioSampleRate);

  osc[0] = SawDetuned(98.1 / 2, context->audioSampleRate);
  for (unsigned int i = 1; i < NUM_VOICES; i++) {
    osc[i] = SawDetuned(98.1 * i, context->audioSampleRate);
  }
  osc[0].setBrightness(0.6);

  // initialize reverb
  for (unsigned int i = 0; i < 2; i++) {
    verb[i] = ZVerb(0.1, 1, context->audioSampleRate);
    dcBlock[i] = new I1P(10.0 / context->audioSampleRate);
  }

  return true;
}

void render(BelaContext *context, void *userData) {
  float beats_old = beats;
  beats += (bpm / 60) * context->audioFrames / context->audioSampleRate;
  if (floor(beats) != floor(beats_old)) {
    // new beat
    rt_printf("beat (%f bpm): %f\n", bpm, beats);
  }

  // process block
  for (unsigned int i = 0; i < 5; i++) {
    osc[i].process_block(context->audioFrames);
  }

  for (unsigned int n = 0; n < context->audioFrames; n++) {
    // float player = 0.2*gPlayer.process();
    for (unsigned int channel = 0; channel < 2; channel++) {
      float out = 0;
      // out += player;
      for (unsigned int i = 0; i < 5; i++) {
        out += osc[i].process(channel);
      }
      // out = verb[channel].process(out);
      out -= dcBlock[channel]->process(out);
      audioWrite(context, n, channel, out);
      if (channel == 0)
        scope.log(out);
    }
  }
}

void cleanup(BelaContext *context, void *userData) {}
