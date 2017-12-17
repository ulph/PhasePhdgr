#pragma once

#include <map>
#include <string>
#include <vector>

#include "ThreadPool.h"

#include "design.hpp"
#include "scope.hpp"

namespace PhasePhckr {
    class GlobalData;
    class SynthVoice;
    class EffectChain;
    class VoiceBus;

    class Effect {
    public:
        Effect();
        virtual ~Effect();
        virtual void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate);

        void handleTimeSignature(int numerator, int denominator);
        void handleBPM(float bpm);
        void handlePosition(float ppqPosition);
        void handleBarPosition(float ppqPosition);
        void handleTime(float time);

        const ParameterHandleMap& setEffectChain(const PatchDescriptor & chain, const ComponentRegister & cp);
        void handleEffectParameter(int handle, float value);
        const Scope& getInputScope(int i) const;
        const Scope& getEffectScope(int i) const;
        static int internalBlockSize();
    protected:
        EffectChain* effects;
        float scopeHz;
        Scope inputScopeL;
        Scope inputScopeR;
        Scope outputScopeL;
        Scope outputScopeR;
        GlobalData *globalData;
    };

    class Synth : public Effect {
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
        const ParameterHandleMap& setVoiceChain(const PatchDescriptor & chain, const ComponentRegister & cp);
        void handleVoiceParameter(int handle, float value);
        const Scope& getVoiceScope(int i) const;
    protected:
        VoiceBus *voiceBus;
        vector<SynthVoice*> voices;
        float lastKnownSampleRate = -1.0f;
        int scopeVoiceIndex;
        Scope voiceScopeL;
        Scope voiceScopeR;
        progschj::ThreadPool pool;
    };
}
