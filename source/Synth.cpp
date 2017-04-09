#include "PhasePhckr.h"
#include "SynthVoice.hpp"
#include "Effect.hpp"

namespace PhasePhckr {

Synth::Synth() {
    for (int i = 0; i<16; ++i) {
        SynthVoiceI* v = new ConnectionGraphVoice();
        v->reset();
        voices.push_back(v);
    }
    voiceBus = VoiceBus(&voices);
}

void Synth::update(float * buffer, int numSamples, float sampleRate)
{
    voiceBus.update();
    for (auto & v : voices) v->update(buffer, numSamples, sampleRate);
    for (auto & e : effects) e->update(buffer, numSamples, sampleRate);
}

size_t Synth::getScopeBuffer(float *buffer, size_t bufferSizeIn) const {
    // here we want to fill in a cycle or two (decimated if needed) 
    // samples worth of the final output ( we know f0 for it )
    float s = 1.f / (float)bufferSizeIn;
    for (int i = 0; i < bufferSizeIn; i++) {
        buffer[i] = (float)i*s;
    }
    return bufferSizeIn;
}

}