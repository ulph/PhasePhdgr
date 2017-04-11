#pragma once

#include <math.h>
#include <vector>
#include <utility>
#include <random>
#include <string.h>
#include <atomic>

namespace PhasePhckr {

    class SynthVoiceI;
    class EffectI;
    struct NoteData;

    struct ChannelData {
      ChannelData() : x(0), y(0), z(0) {}
        float x;
        float y;
        float z;
    };

    struct GlobalData {
        GlobalData() : exp(0), brt(0), mod(0) {}
        float exp;
        float brt;
        float mod;
    };

    class VoiceBus {
        // TODO, this class needs to be split into 2 (or more) classes
        // only the handleBLABLA stuff should be visible on api level
    public:
        VoiceBus() : voices(nullptr), globalDataSlewFactor(0.995f) {}
        VoiceBus(std::vector<SynthVoiceI*> * parent_voices);
        virtual ~VoiceBus();
        void handleNoteOnOff(int channel, int note, float velocity, bool on);
        void handleX(int channel, float position);
        void handleY(int channel, float position);
        void handleZ(int channel, float position);
        void handleNoteZ(int channel, int note, float position);
        void handleExpression(float value);
        void handleBreath(float value);
        void handleModWheel(float value);
        void update();
        float findScopeVoiceHz();
        const GlobalData& getGlobalData();
    private:
        std::vector<SynthVoiceI*> *voices;
        std::vector<NoteData*> notes;
        int getNoteDataIndex(int channel, int note);
        int findYoungestInactiveNoteDataIndex(int channel);
        ChannelData channelData[16];
        GlobalData globalData;
        GlobalData globalDataTarget;
        float globalDataSlewFactor;
    };

    class AutomationBus {
        // handles the automation (global VST) parameters
    };

    class Synth {
    public:
        // TODO, encapsulate the scope stuff into it's own class so we can have several
        Synth();
        VoiceBus voiceBus;
        AutomationBus automationBus;
        virtual void update(float * buffer, int numSamples, float sampleRate);
        virtual size_t getScopeBuffer(float *buffer, size_t bufferSizeIn) const;
    private:
        std::vector<SynthVoiceI*> voices; // per note sound generation
        EffectI* effects; // effects applied to mix of voices (in series)
        // settings like voice stacking, per voice detuning and etc etc goes here
        float scopeBuffer[512];
        const unsigned int scopeBufferSize;
        unsigned int scopeBufferWriteIndex;
        float scopeDrift;
    };

}
