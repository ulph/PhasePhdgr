#pragma once

#include <math.h>
#include <vector>
#include <utility>
#include <random>
#include <string.h>
#include <atomic>

namespace PhasePhckr {

    const float c_slewFactor = 0.995f;

    class SynthVoice;
    class EffectChain;
    class VoiceBus;
    class GlobalData;
    struct ConnectionGraphDescriptor;
    struct ComponentDescriptor;
    class ComponentRegister;

    const ConnectionGraphDescriptor& getExampleFxChain();
    const ConnectionGraphDescriptor& getExampleVoiceChain();

    class Scope {
    private:
        float scopeBuffer[512];
        const size_t scopeBufferSize;
        unsigned int scopeBufferWriteIndex;
        float scopeDrift;
    public:
        Scope();
        const float* getBuffer(int* size) const;
        void writeToBuffer(const float * leftChannelbuffer, int numSamples, float sampleRate, float hz);
    };

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
        void setFxChain(const ConnectionGraphDescriptor & fxChain, const ComponentRegister & cp);
        void setVoiceChain(const ConnectionGraphDescriptor & fxChain, const ComponentRegister & cp);
        const Scope& getVoiceScope(int i) const {
            if(i==0)
                return voiceScopeL;
            if(i==1)
                return voiceScopeR;
            return voiceScopeL;
        }
        const Scope& getOutputScope(int i) const {
            if(i==0)
                return outputScopeL;
            if(i==1)
                return outputScopeR;
            return outputScopeL;
        }
    private:
        VoiceBus *voiceBus;
        std::vector<SynthVoice*> voices;
        EffectChain* effects;
        Scope voiceScopeL;
        Scope voiceScopeR;
        Scope outputScopeL;
        Scope outputScopeR;
        GlobalData *globalData;
        float scopeHz;
        int scopeVoiceIndex;
    };

}
