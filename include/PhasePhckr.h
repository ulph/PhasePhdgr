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
    struct ConnectionGraphDescriptor_Numerical;

    const ConnectionGraphDescriptor& getExampleFxChain();
    const ConnectionGraphDescriptor_Numerical& getExampleVoiceChain();

    class Scope {
    private:
        float scopeBuffer[512];
        const size_t scopeBufferSize;
        unsigned int scopeBufferWriteIndex;
        float scopeDrift;
    public:
        Scope();
        const float* Scope::getBuffer(int* size) const;
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
        void setFxChain(const ConnectionGraphDescriptor& fxChain);
        void setVoiceChain(const ConnectionGraphDescriptor_Numerical& fxChain);
        const Scope& getVoiceScope() const { return voiceScope; }
        const Scope& getOutputScope() const { return outputScope; }
    private:
        VoiceBus *voiceBus;
        std::vector<SynthVoice*> voices;
        EffectChain* effects;
        Scope voiceScope;
        Scope outputScope;
        GlobalData *globalData;
        float scopeHz;
        int scopeVoiceIndex;
    };

}
