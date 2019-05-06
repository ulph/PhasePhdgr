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

    class SDKExtensionManager;

    class Base {
    public:
        Base();
        virtual ~Base();
        virtual void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate) = 0;
        virtual const ParameterHandleMap& setPatch(const PatchDescriptor & chain, const ComponentRegister & cp) = 0;
        virtual const ParameterHandleMap& setPatch(const PatchDescriptor & chain, const ComponentRegister & cp, const SDKExtensionManager & sdk) = 0;
        void handleTimeSignature(int numerator, int denominator);
        void handleBPM(float bpm);
        void handlePosition(float ppqPosition);
        void handleBarPosition(float ppqPosition);
        void handleTime(float time);
        virtual void applySettings(const PresetSettings& settings) {};
        static int internalBlockSize();
        virtual const Scope& getVoiceScope(int i) const;
        virtual const Scope& getInputScope(int i) const;
        virtual const Scope& getOutputScope(int i) const;
        virtual void handleParameter(int handle, float value) = 0;
    protected:
        float scopeHz;
        GlobalData * globalData;
        Scope outputScopeL;
        Scope outputScopeR;
        float secondsSinceReset = 0;
    };

    class Effect : public Base {
    public:
        Effect();
        virtual ~Effect();
        void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate) override;
        const ParameterHandleMap& setPatch(const PatchDescriptor & chain, const ComponentRegister & cp) override;
        const ParameterHandleMap& setPatch(const PatchDescriptor & chain, const ComponentRegister & cp, const SDKExtensionManager & sdk) override;
        const Scope& getInputScope(int i) const override;
        void handleParameter(int handle, float value) override;
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
        const ParameterHandleMap& setPatch(const PatchDescriptor & chain, const ComponentRegister & cp, const SDKExtensionManager & sdk) override;
        void handleParameter(int handle, float value) override;
        const Scope& getVoiceScope(int i) const override;
        void applySettings(const PresetSettings& settings) override;
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

    class PluginsRegister;
    class Doc;
    class SDKExtensionManager {
    public:
        SDKExtensionManager();
        virtual ~SDKExtensionManager();
        void registerSdkExtensions(const std::set<std::string>& filenames);
        void updateDoc(Doc* doc);
        friend Base;
        friend Effect;
        friend Synth;
    private:
        PluginsRegister * sdkPluginRegister = nullptr;
    };

}
