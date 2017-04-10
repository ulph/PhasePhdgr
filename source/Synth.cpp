#include "PhasePhckr.h"
#include "SynthVoice.hpp"
#include "Effect.hpp"

namespace PhasePhckr {

const float preSaturationReductionFactor = 0.75;
const float postSaturationReductionFactor = 1.f/atanf(2.0f);

Synth::Synth() 
    : scopeBufferWriteIndex(0), 
    scopeBufferSize(sizeof(scopeBuffer)/sizeof(float))
{
    for (int i = 0; i<16; ++i) {
        SynthVoiceI* v = new ConnectionGraphVoice();
        v->reset();
        voices.push_back(v);
    }
    voiceBus = VoiceBus(&voices);
    memset(scopeBuffer, 0, sizeof(scopeBuffer));
}

void Synth::update(float * buffer, int numSamples, float sampleRate)
{
    voiceBus.update();
    for (auto & v : voices) v->update(buffer, numSamples, sampleRate);
    for (auto & e : effects) e->update(buffer, numSamples, sampleRate);

    // apply a silly gain stage to tame the overloads
    for(int i=0; i<numSamples; i++){
        buffer[i] = atanf(preSaturationReductionFactor*buffer[i]) * postSaturationReductionFactor;
    }

    // find the "active" voice's hz
    float hz = voiceBus.findScopeVoiceHz();

    // fill scope buffer with a (poorly) resampled version matching a couple of cycles
    if(hz > 1){
        float numPeriods = 1;
        float samplesPerPeriod = sampleRate / hz;
        float decimation = numPeriods * samplesPerPeriod / (float)scopeBufferSize;
        for(float i=0; i<numSamples; i+=decimation){
            scopeBuffer[scopeBufferWriteIndex] = buffer[(unsigned int)i];
            scopeBufferWriteIndex++;
            scopeBufferWriteIndex %= scopeBufferSize;
        }
    }
    else{
        scopeBufferWriteIndex = 0;
        memset(scopeBuffer, 0, sizeof(scopeBuffer));
    }
}

size_t Synth::getScopeBuffer(float *buffer, size_t bufferSizeIn) const 
{
    int size = bufferSizeIn > scopeBufferSize ? scopeBufferSize:bufferSizeIn;
    for (int i = 0; i < size; i++) {
        buffer[i] = scopeBuffer[i];
    }
    return size;
}

}