#include "PhasePhckr.h"
#include "SynthVoice.hpp"
#include "EffectChain.hpp"

namespace PhasePhckr {

Synth::Synth() 
    : effects(new EffectChain())
{
    for (int i = 0; i<16; ++i) {
        SynthVoice* v = new SynthVoice();
        voices.push_back(v);
    }
    voiceBus = new VoiceBus(&voices);
}

Synth::~Synth(){
    for(SynthVoice *v : voices){
        delete v;
    }
    delete effects;
    delete voiceBus;
}

void Synth::update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate)
{
    voiceBus->update();

    int samplesLeft = numSamples;
    int maxChunk = SYNTH_VOICE_BUFFER_LENGTH;
    float *bufL = leftChannelbuffer, *bufR = rightChannelbuffer;

    while(samplesLeft > 0) {
        int chunkSize = (samplesLeft < maxChunk) ? samplesLeft : maxChunk;

        for(auto & v : voices) v->processingStart(chunkSize, sampleRate, voiceBus->getGlobalData());
        for(auto & v : voices) v->processingFinish(bufL, bufR, chunkSize);

        samplesLeft -= chunkSize;
        bufL += chunkSize;
        bufR += chunkSize;
    }

    effects->update(leftChannelbuffer, rightChannelbuffer, numSamples, sampleRate, voiceBus->getGlobalData());

    scope.writeToBuffer(leftChannelbuffer, numSamples, sampleRate, voiceBus->findScopeVoiceHz());
}

void Synth::handleNoteOnOff(int a, int b, float c, bool d) { voiceBus->handleNoteOnOff(a, b, c, d); }
void Synth::handleX(int a, float b) { voiceBus->handleX(a, b); }
void Synth::handleY(int a, float b) { voiceBus->handleY(a, b); }
void Synth::handleZ(int a, float b) { voiceBus->handleZ(a, b); }
void Synth::handleNoteZ(int a, int b, float c) { voiceBus->handleNoteZ(a, b, c); }
void Synth::handleExpression(float a) { voiceBus->handleExpression(a); }
void Synth::handleBreath(float a) { voiceBus->handleBreath(a); }
void Synth::handleModWheel(float a) { voiceBus->handleModWheel(a); }

Scope::Scope()
    : scopeBufferWriteIndex(0)
    , scopeBufferSize(sizeof(scopeBuffer) / sizeof(float))
    , scopeDrift(0.0f)
{
    memset(scopeBuffer, 0, sizeof(scopeBuffer));
}

void Scope::writeToBuffer(float * sourceBuffer, int numSamples, float sampleRate, float hz) {
    // fill scope buffer with a (poorly) resampled version matching a couple of cycles
    if (hz > 1) {
        unsigned int numPeriods = 2;
        float samplesPerPeriod = sampleRate / hz;
        float decimation = (float)numPeriods * samplesPerPeriod / (float)scopeBufferSize;
        float i = 0;
        for (i = 0; i<numSamples; i += decimation) {
            scopeBuffer[scopeBufferWriteIndex] = sourceBuffer[(unsigned int)i];
            scopeBufferWriteIndex++;
            scopeBufferWriteIndex %= scopeBufferSize;
        }

        // compensate for rounding errors (aka, drift)
        scopeDrift += ((float)numSamples - i) / decimation;
        if (scopeDrift >= 1) {
            scopeDrift--;
            scopeBuffer[scopeBufferWriteIndex] = sourceBuffer[numSamples - 1];
            scopeBufferWriteIndex++;
            scopeBufferWriteIndex %= scopeBufferSize;
        }
        else if (scopeDrift <= -1) {
            scopeDrift++;
            scopeBufferWriteIndex--;
            scopeBufferWriteIndex %= scopeBufferSize;
        }
    }
    else {
        scopeBufferWriteIndex = 0;
        memset(scopeBuffer, 0, sizeof(scopeBuffer));
    }
}

size_t Scope::copyBuffer(float *buffer, size_t bufferSizeIn) const
{
    size_t size = bufferSizeIn > scopeBufferSize ? scopeBufferSize : bufferSizeIn;
    for (int i = 0; i < size; i++) {
        buffer[i] = scopeBuffer[i];
    }
    return size;
}

}
