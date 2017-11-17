#pragma once

#include <map>
#include <string>

#include "design.hpp"
#include "scope.hpp"

namespace PhasePhckr {
    class GlobalData;
    class SynthVoice;
    class EffectChain;
    class VoiceBus;

    class Synth {
    public:
        Synth();
        virtual ~Synth();
        virtual void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate);
        void handleNoteOnOff(int channel, int note, float velocity, bool on);
        void handleX(int channel, float position);
        void handleY(int channel, float position);
        void handleZ(int channel, float position);
        void handleNoteZ(int channel, int note, float position);
        void handleExpression(float value);
        void handleBreath(float value);
        void handleModWheel(float value);
        void handleTimeSignature(int numerator, int denominator);
        void handleBPM(float bpm);
        void handlePosition(float ppqPosition);
        void handleTime(float time);
        const ParameterHandleMap& setEffectChain(const PatchDescriptor & chain, const ComponentRegister & cp);
        const ParameterHandleMap& setVoiceChain(const PatchDescriptor & chain, const ComponentRegister & cp);
        void handleEffectParameter(int handle, float value);
        void handleVoiceParameter(int handle, float value);
        const Scope& getVoiceScope(int i) const;
        const Scope& getOutputScope(int i) const;
    private:
        VoiceBus *voiceBus;
        vector<SynthVoice*> voices;
        EffectChain* effects;
        float scopeHz;
        int scopeVoiceIndex;
        Scope voiceScopeL;
        Scope voiceScopeR;
        Scope outputScopeL;
        Scope outputScopeR;
        GlobalData *globalData;
    };
}