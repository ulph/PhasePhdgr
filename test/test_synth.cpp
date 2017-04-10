#include <stdio.h>

#include <functional>
#include <vector>
#include <tuple>

#include "PhasePhckr.h"

static const float kSamplerate = 48000.f;
static const int kNumSamples = 64;

int main(int argc, char **argv)
{
  PhasePhckr::Synth synth;
  float time = 0;
  float buffer[kNumSamples];

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

  std::vector< std::tuple<float, std::function<void()> > > piano_roll;

  piano_roll.emplace_back(0 * beat, std::bind(&PhasePhckr::VoiceBus::handleNoteOnOff, &synth.voiceBus, 0, 64, 1.0f, true));
  piano_roll.emplace_back(0 * beat, std::bind(&PhasePhckr::VoiceBus::handleZ, &synth.voiceBus, 0, .5f));
  piano_roll.emplace_back(1 * beat, std::bind(&PhasePhckr::VoiceBus::handleNoteOnOff, &synth.voiceBus, 0, 64, 0.f, false));

  piano_roll.emplace_back(2 * beat, std::bind(&PhasePhckr::VoiceBus::handleNoteOnOff, &synth.voiceBus, 1, 78, 1.0f, true));
  piano_roll.emplace_back(2 * beat, std::bind(&PhasePhckr::VoiceBus::handleX, &synth.voiceBus, 1, 0.f));
  piano_roll.emplace_back(2 * beat, std::bind(&PhasePhckr::VoiceBus::handleZ, &synth.voiceBus, 1, .5f));
  piano_roll.emplace_back(2.25 * beat, std::bind(&PhasePhckr::VoiceBus::handleX, &synth.voiceBus, 1, -.25f));
  piano_roll.emplace_back(2.5 * beat, std::bind(&PhasePhckr::VoiceBus::handleX, &synth.voiceBus, 1, -.5f));
  piano_roll.emplace_back(2.75 * beat, std::bind(&PhasePhckr::VoiceBus::handleX, &synth.voiceBus, 1, -.75f));
  piano_roll.emplace_back(3 * beat, std::bind(&PhasePhckr::VoiceBus::handleX, &synth.voiceBus, 1, -1.f));
  piano_roll.emplace_back(3 * beat, std::bind(&PhasePhckr::VoiceBus::handleNoteOnOff, &synth.voiceBus, 1, 78, .7f, false));

  auto it = piano_roll.begin();
  while(time < 3.)
  {
    memset(buffer, 0, sizeof(buffer));
    while(it != piano_roll.end() && std::get<0>(*it) < time)
    {
      std::get<1>(*it)();
      it++;
    }


    synth.update(buffer, kNumSamples, kSamplerate);
    fwrite(buffer, sizeof(buffer), 1, out_fid);
    time += kNumSamples / kSamplerate;
  }

  if(should_close_out_fid)
    fclose(out_fid);
}
