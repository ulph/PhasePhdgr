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

    class Base {
    public:
        Base();
        virtual ~Base();
        virtual void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate) = 0;
        virtual const ParameterHandleMap& setPatch(const PatchDescriptor & chain, const ComponentRegister & cp) = 0;
        void handleTimeSignature(int numerator, int denominator);
        void handleBPM(float bpm);
        void handlePosition(float ppqPosition);
        void handleBarPosition(float ppqPosition);
        void handleTime(float time);
        static int internalBlockSize();
        const Scope& getOutputScope(int i) const;
        virtual void handleParameter(int handle, float value) = 0;
    protected:
        float scopeHz;
        GlobalData * globalData;
        Scope outputScopeL;
        Scope outputScopeR;
    };

    class Effect : public Base {
    public:
        Effect();
        virtual ~Effect();
        void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate) override;
        const ParameterHandleMap& setPatch(const PatchDescriptor & chain, const ComponentRegister & cp) override;
        const Scope& getInputScope(int i) const;
        virtual void handleParameter(int handle, float value);
        float setScopeHz(float hz) { return scopeHz = hz; }
    protected:
        EffectChain* effects;
        Scope inputScopeL;
        Scope inputScopeR;
    };

    class Synth : public Base {
    public:
        Synth();
        virtual ~Synth();
        void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate) override;
        void handleNoteOnOff(int channel, int note, float velocity, bool on);
        void handleX(int channel, float position);
        void handleY(int channel, float position);
        void handleZ(int channel, float position);
        void handleNoteZ(int channel, int note, float position);
        void handleSustain(float position);
        void handleExpression(float value);
        void handleBreath(float value);
        void handleModWheel(float value);
        const ParameterHandleMap& setPatch(const PatchDescriptor & chain, const ComponentRegister & cp) override;
        virtual void handleParameter(int handle, float value);
        const Scope& getVoiceScope(int i) const;
        void applySettings(const PresetSettings& settings);
        const PresetSettings& retrieveSettings();
        float getScopeHz() { return scopeHz; }
    protected:
        VoiceBus *voiceBus;
        vector<SynthVoice*> voices;
        void resetVoiceBus(const SynthVoice* voice);
        float lastKnownSampleRate = -1.0f;
        int scopeVoiceIndex;
        Scope voiceScopeL;
        Scope voiceScopeR;
        size_t concurrency;
        PresetSettings settings;
        progschj::ThreadPool pool;
    };
}
