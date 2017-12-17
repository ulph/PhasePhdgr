#include <phasephckr/synth.hpp>

#include "synthvoice.hpp"
#include "effectchain.hpp"

namespace PhasePhckr {

const size_t numVoices = 16;

Effect::Effect()
    : effects(nullptr)
    , globalData(new GlobalData())
    , scopeHz(10.f)
{
}

Effect::~Effect() {
    delete effects;
    delete globalData;
}

Synth::Synth()
    : voiceBus(new VoiceBus())
    , scopeVoiceIndex(-1)
    , pool(std::thread::hardware_concurrency()) 
{
}

Synth::~Synth(){
    for(SynthVoice *v : voices){
        delete v;
    }
    delete voiceBus;
}

int Effect::internalBlockSize() {
    return ConnectionGraph::k_blockSize;
}

const ParameterHandleMap& Effect::setEffectChain(const PatchDescriptor& fxChain, const ComponentRegister & cp) {
    delete effects;
    effects = new EffectChain(fxChain, cp);
    return effects->getParameterHandles();
}

const ParameterHandleMap& Synth::setVoiceChain(const PatchDescriptor& voiceChain, const ComponentRegister & cp){
    assert(numVoices > 0);
    delete voiceBus;
    voiceBus = new VoiceBus();
    for (SynthVoice *v : voices) {
        delete v;
    }
    voices.clear();
    SynthVoice v(voiceChain, cp);
    for (int i = 0; i<numVoices; ++i) {
        SynthVoice *v_ = new SynthVoice(v);
        v_->mpe.setIndex(i, numVoices);
        voices.push_back(v_);
        v_->preCompile(lastKnownSampleRate);
    }
    return voices[0]->getParameterHandles(); // they're identical
}

void Effect::update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate)
{
    inputScopeL.writeToBuffer(leftChannelbuffer, numSamples, sampleRate, scopeHz);
    inputScopeR.writeToBuffer(rightChannelbuffer, numSamples, sampleRate, scopeHz);
    effects->update(leftChannelbuffer, rightChannelbuffer, numSamples, sampleRate, *globalData);
    outputScopeL.writeToBuffer(leftChannelbuffer, numSamples, sampleRate, scopeHz);
    outputScopeR.writeToBuffer(rightChannelbuffer, numSamples, sampleRate, scopeHz);
}

void Synth::update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate)
{
    if (sampleRate != lastKnownSampleRate) {
        for (auto* v : voices) {
            v->preCompile(sampleRate);
        }
    }
    lastKnownSampleRate = sampleRate;

    int samplesLeft = numSamples;
    int maxChunk = SYNTH_VOICE_BUFFER_LENGTH;
    float *bufL = leftChannelbuffer, *bufR = rightChannelbuffer;

    int newScopeVoiceIndex = voiceBus->findScopeVoiceIndex(voices);
    if (newScopeVoiceIndex != -1) {
        scopeVoiceIndex = newScopeVoiceIndex;
        scopeHz = voices[scopeVoiceIndex]->mpe.getState().pitchHzTarget;
    }
    else if(scopeVoiceIndex != -1 && voices[scopeVoiceIndex]->isSilent()){
        scopeVoiceIndex = -1;
    }

    while(samplesLeft > 0) {
        int chunkSize = (samplesLeft < maxChunk) ? samplesLeft : maxChunk;

        for (auto & v : voices) v->processingStart(chunkSize, sampleRate, *globalData);

        std::vector< std::future<void> > results;
        for (auto & v : voices) {
            results.emplace_back(
                pool.enqueue([v]{v->threadedProcess();})
            );
        }
        for (auto && result : results) result.get(); // get all the futures

        for (auto & v : voices) v->processingFinish(bufL, bufR, chunkSize);

        samplesLeft -= chunkSize;
        bufL += chunkSize;
        bufR += chunkSize;

        if (scopeVoiceIndex != -1) {
            voiceScopeL.writeToBuffer(voices[scopeVoiceIndex]->getInternalBuffer(0), chunkSize, sampleRate, scopeHz);
            voiceScopeR.writeToBuffer(voices[scopeVoiceIndex]->getInternalBuffer(1), chunkSize, sampleRate, scopeHz);
        }
    }

    effects->update(leftChannelbuffer, rightChannelbuffer, numSamples, sampleRate, *globalData);

    outputScopeL.writeToBuffer(leftChannelbuffer, numSamples, sampleRate, scopeHz);
    outputScopeR.writeToBuffer(rightChannelbuffer, numSamples, sampleRate, scopeHz);

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
void Effect::handleTimeSignature(int num, int den){ globalData->signature(num, den); }
void Effect::handleBPM(float bpm){ globalData->bpm(bpm); }
void Effect::handlePosition(float ppqPosition){ globalData->position(ppqPosition); }
void Effect::handleBarPosition(float ppqPosition){ globalData->barPosition(ppqPosition); }

void Effect::handleTime(float t){ globalData->time(t); }

void Effect::handleEffectParameter(int handle, float value){
    effects->setParameter(handle, value);
}

void Synth::handleVoiceParameter(int handle, float value){
    for(auto & v : voices) v->setParameter(handle, value);
}

const Scope& Synth::getVoiceScope(int i) const {
    if(i==0) return voiceScopeL;
    else if(i==1) return voiceScopeR;
    return voiceScopeL;
}

const Scope& Effect::getEffectScope(int i) const {
    if(i==0) return outputScopeL;
    else if(i==1) return outputScopeR;
    return outputScopeL;
}

const Scope& Effect::getInputScope(int i) const {
    if (i == 0) return inputScopeL;
    else if (i == 1) return inputScopeR;
    return inputScopeL;
}

}
