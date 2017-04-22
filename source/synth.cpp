#include "phasephckr.h"
#include "synthvoice.hpp"
#include "effectchain.hpp"

namespace PhasePhckr {

const size_t numVoices = 16;

Synth::Synth()
    : effects(nullptr)
    , globalData(new GlobalData())
    , voiceBus(new VoiceBus())
    , scopeHz(0)
    , scopeVoiceIndex(-1)
{
}

Synth::~Synth(){
    for(SynthVoice *v : voices){
        delete v;
    }
    delete effects;
    delete voiceBus;
    delete globalData;
}

void Synth::setFxChain(const ConnectionGraphDescriptor& fxChain) {
    delete effects;
    effects = new EffectChain(fxChain);
}

template <class T>
void Synth::setVoiceChain_internal(const T& voiceChain) {
    delete voiceBus;
    voiceBus = new VoiceBus();
    for (SynthVoice *v : voices) {
        delete v;
    }
    voices.clear();
    for (int i = 0; i<numVoices; ++i) {
        SynthVoice* v = new SynthVoice(voiceChain);
        voices.push_back(v);
    }
}
void Synth::setVoiceChain(const ConnectionGraphDescriptor& voiceChain){
    setVoiceChain_internal(voiceChain);
}

void Synth::setVoiceChain(const ConnectionGraphDescriptor_Numerical& voiceChain){
    setVoiceChain_internal(voiceChain);
}

void Synth::update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate)
{
    int samplesLeft = numSamples;
    int maxChunk = SYNTH_VOICE_BUFFER_LENGTH;
    float *bufL = leftChannelbuffer, *bufR = rightChannelbuffer;

    while(samplesLeft > 0) {
        int chunkSize = (samplesLeft < maxChunk) ? samplesLeft : maxChunk;

        for(auto & v : voices) v->processingStart(chunkSize, sampleRate, *globalData);
        for(auto & v : voices) v->processingFinish(bufL, bufR, chunkSize);

        samplesLeft -= chunkSize;
        bufL += chunkSize;
        bufR += chunkSize;

        // fill the voice scope buffer with output of selected voice
        int nextScopeVoiceIndex = voiceBus->findScopeVoiceIndex(voices);
        if (nextScopeVoiceIndex != -1) {
            scopeVoiceIndex = nextScopeVoiceIndex;
            scopeHz = voices[scopeVoiceIndex]->mpe.getState().pitchHz;
        }
        if (scopeVoiceIndex != -1) {
            voiceScopeL.writeToBuffer(voices[scopeVoiceIndex]->getInternalBuffer(0), chunkSize, sampleRate, scopeHz);
            voiceScopeR.writeToBuffer(voices[scopeVoiceIndex]->getInternalBuffer(1), chunkSize, sampleRate, scopeHz);
        }
    }

    if (effects) {
        effects->update(leftChannelbuffer, rightChannelbuffer, numSamples, sampleRate, *globalData);
    }

    outputScopeL.writeToBuffer(leftChannelbuffer, numSamples, sampleRate, scopeHz);
    outputScopeR.writeToBuffer(rightChannelbuffer, numSamples, sampleRate, scopeHz);

    for (int i = 0; i < numSamples; i++) {
        globalData->update();
    }

    voiceBus->update();

}

void Synth::handleNoteOnOff(int a, int b, float c, bool d) { voiceBus->handleNoteOnOff(a, b, c, d, voices); }
void Synth::handleX(int a, float b) { voiceBus->handleX(a, b, voices); }
void Synth::handleY(int a, float b) { voiceBus->handleY(a, b, voices); }
void Synth::handleZ(int a, float b) { voiceBus->handleZ(a, b, voices); }
void Synth::handleNoteZ(int a, int b, float c) { voiceBus->handleNoteZ(a, b, c, voices); }
void Synth::handleExpression(float a) { globalData->expression(a); }
void Synth::handleBreath(float a) { globalData->breath(a); }
void Synth::handleModWheel(float a) { globalData->modwheel(a); }

}
