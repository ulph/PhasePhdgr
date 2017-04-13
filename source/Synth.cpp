#include "PhasePhckr.h"
#include "SynthVoice.hpp"
#include "EffectChain.hpp"

namespace PhasePhckr {

const size_t numVoices = 16;

Synth::Synth()
    : effects(nullptr)
    , globalData(new GlobalData())
    , voiceBus(new VoiceBus())
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

void Synth::setVoiceChain(const ConnectionGraphDescriptor_Numerical& voiceChain) {
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
    }

    if (effects) {
        effects->update(leftChannelbuffer, rightChannelbuffer, numSamples, sampleRate, *globalData);
    }

    for (int i = 0; i < numSamples; i++) {
        globalData->update();
    }

    voiceBus->update();

    scope.writeToBuffer(leftChannelbuffer, numSamples, sampleRate, voiceBus->findScopeVoiceHz(voices));
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
