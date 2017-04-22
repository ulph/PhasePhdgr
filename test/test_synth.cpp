#include <stdio.h>

#include <functional>
#include <vector>
#include <utility>

#include "phasephckr.h"
#include "EffectChain.hpp"

static const float kSamplerate = 48000.f;
static const int kNumSamples = 64;

int main(int argc, char **argv)
{
  PhasePhckr::Synth synth;
  synth.setFxChain(PhasePhckr::getExampleFxChain());
  synth.setVoiceChain(PhasePhckr::getExampleVoiceChain());
  float time = 0;
  float bufferL[kNumSamples];
  float bufferR[kNumSamples];

  float tempo = 120.f;
  float beat = 60. / tempo;

  FILE *out_fid = nullptr;
  bool should_close_out_fid = false;
  if(argc > 1)
  {
    out_fid = fopen(argv[1], "wb");
    if(out_fid){
        should_close_out_fid = true;
    }
  }

  if(out_fid == nullptr){
    out_fid = stdout;
  }

  std::vector< std::pair<float, std::function<void()> > > piano_roll;

  piano_roll.emplace_back(0 * beat, std::bind(&PhasePhckr::Synth::handleNoteOnOff, &synth, 0, 64, 1.0f, true));
  piano_roll.emplace_back(0 * beat, std::bind(&PhasePhckr::Synth::handleZ, &synth, 0, .5f));
  piano_roll.emplace_back(1 * beat, std::bind(&PhasePhckr::Synth::handleNoteOnOff, &synth, 0, 64, 0.f, false));

  piano_roll.emplace_back(2 * beat, std::bind(&PhasePhckr::Synth::handleNoteOnOff, &synth, 1, 78, 1.0f, true));
  piano_roll.emplace_back(2 * beat, std::bind(&PhasePhckr::Synth::handleX, &synth, 1, 0.f));
  piano_roll.emplace_back(2 * beat, std::bind(&PhasePhckr::Synth::handleZ, &synth, 1, .5f));
  piano_roll.emplace_back(2.25 * beat, std::bind(&PhasePhckr::Synth::handleX, &synth, 1, -.25f));
  piano_roll.emplace_back(2.5 * beat, std::bind(&PhasePhckr::Synth::handleX, &synth, 1, -.5f));
  piano_roll.emplace_back(2.75 * beat, std::bind(&PhasePhckr::Synth::handleX, &synth, 1, -.75f));
  piano_roll.emplace_back(3 * beat, std::bind(&PhasePhckr::Synth::handleX, &synth, 1, -1.f));
  piano_roll.emplace_back(3 * beat, std::bind(&PhasePhckr::Synth::handleNoteOnOff, &synth, 1, 78, .7f, false));

  auto it = piano_roll.begin();
  while(time < 3.)
  {
    memset(bufferL, 0, sizeof(bufferL));
    memset(bufferR, 0, sizeof(bufferR));

    while(it != piano_roll.end() && it->first < time)
    {
      it->second();
      it++;
    }


    synth.update(bufferL, bufferR, kNumSamples, kSamplerate);
    fwrite(bufferL, sizeof(bufferL), 1, out_fid);
    time += kNumSamples / kSamplerate;
  }

  if(should_close_out_fid)
    fclose(out_fid);
}

