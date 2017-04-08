#include "PhasePhckr.h"
#include "SynthVoice.hpp"
#include "Effect.hpp"

namespace PhasePhckr {

Synth::Synth() {
    for (int i = 0; i<16; ++i) {
        SynthVoiceI* v = new ExConnectionGraphVoice();
        v->reset();
        voices.push_back(v);
    }
    voiceBus = VoiceBus(&voices);
}

void Synth::update(float * buffer, int numSamples, float sampleRate)
{
    for (auto & v : voices) v->update(buffer, numSamples, sampleRate);
    for (auto & e : effects) e->update(buffer, numSamples, sampleRate);
}

}