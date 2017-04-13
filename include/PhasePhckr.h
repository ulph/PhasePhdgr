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
        size_t copyBuffer(float *buffer, size_t bufferSizeIn) const;
        void writeToBuffer(float * leftChannelbuffer, int numSamples, float sampleRate, float hz);
    };

    class Synth {
    public:
        Synth(const ConnectionGraphDescriptor& fxChain);
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
        const Scope& getScope() const { return scope; }
    private:
        VoiceBus *voiceBus;
        std::vector<SynthVoice*> voices;
        EffectChain* effects;
        Scope scope;
        GlobalData *globalData;
    };

}
