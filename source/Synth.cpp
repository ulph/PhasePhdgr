#include "PhasePhckr.h"
#include "SynthVoice.hpp"
#include "EffectChain.hpp"

namespace PhasePhckr {

Synth::Synth() 
    : effects(new EffectChain())
    , globalData(new GlobalData())
    , voiceBus(new VoiceBus())
{
    for (int i = 0; i<16; ++i) {
        SynthVoice* v = new SynthVoice();
        voices.push_back(v);
    }
}

Synth::~Synth(){
    for(SynthVoice *v : voices){
        delete v;
    }
    delete effects;
    delete voiceBus;
    delete globalData;
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

    effects->update(leftChannelbuffer, rightChannelbuffer, numSamples, sampleRate, *globalData);

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
