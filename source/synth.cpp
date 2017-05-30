#include "phasephckr.h"
#include "synthvoice.hpp"
#include "effectchain.hpp"

namespace PhasePhckr {

const size_t numVoices = 16;

Synth::Synth()
    : voiceBus(new VoiceBus())
    , effects(nullptr)
    , globalData(new GlobalData())
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

const map<string, int>& Synth::setFxChain(const PatchDescriptor& fxChain, const ComponentRegister & cp) {
    delete effects;
    effects = new EffectChain(fxChain, cp);
    return effects->getParameterHandles();
}

const map<string, int>& Synth::setVoiceChain(const PatchDescriptor& voiceChain, const ComponentRegister & cp){
    assert(numVoices > 0);
    delete voiceBus;
    voiceBus = new VoiceBus();
    for (SynthVoice *v : voices) {
        delete v;
    }
    voices.clear();
    for (int i = 0; i<numVoices; ++i) {
        SynthVoice* v = new SynthVoice(voiceChain, cp);
        voices.push_back(v);
    }
    return voices[0]->getParameterHandles(); // they're identical
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
void Synth::handleTimeSignature(int num, int den){ globalData->signature(num, den); }
void Synth::handleBPM(float bpm){ globalData->bpm(bpm); }
void Synth::handlePosition(float pos){ globalData->position(pos); }
void Synth::handleTime(float t){ globalData->time(t); }

void Synth::setFxParameter(int handle, float value){
    effects->setParameter(handle, value);
}

void Synth::setVoiceParameter(int handle, float value){
    for(auto & v : voices) v->setParameter(handle, value);
}

const Scope& Synth::getVoiceScope(int i) const {
    if(i==0) return voiceScopeL;
    if(i==1) return voiceScopeR;
    return voiceScopeL;
}

const Scope& Synth::getOutputScope(int i) const {
    if(i==0) return outputScopeL;
    if(i==1) return outputScopeR;
    return outputScopeL;
}

}
