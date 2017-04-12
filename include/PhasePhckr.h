#pragma once

#include <math.h>
#include <vector>
#include <utility>
#include <random>
#include <string.h>
#include <atomic>

namespace PhasePhckr {

    const float c_slewFactor = 0.995;

    class SynthVoice;
    class EffectChain;
    struct NoteData;

    struct ChannelData {
      ChannelData() : x(0), y(0), z(0) {}
        float x;
        float y;
        float z;
    };

    struct GlobalDataState {
        GlobalDataState() : exp(0), brt(0), mod(0) {}
        float exp;
        float brt;
        float mod;
    };

    class GlobalData {
    private:
        GlobalDataState tg;
        GlobalDataState st;
        float slewFactor;
    public:
        void modwheel(float v) { tg.mod = v; }
        void expression(float v) { tg.exp = v; }
        void breath(float v) { tg.brt = v; }
        GlobalData() : slewFactor(c_slewFactor) {}
        void update() {
            st.mod = slewFactor * st.mod + (1.0f - slewFactor) * tg.mod;
            st.exp = slewFactor * st.exp + (1.0f - slewFactor) * tg.exp;
            st.brt = slewFactor * st.brt + (1.0f - slewFactor) * tg.brt;
        }
        const GlobalDataState & getState() { return st; }
    };

    class VoiceBus {
        // TODO, this class needs to be split into 2 (or more) classes
        // only the handleBLABLA stuff should be visible on api level
    public:
        VoiceBus() : voices(nullptr) {}
        VoiceBus(std::vector<SynthVoice*> * parent_voices);
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
        const GlobalData& getGlobalData() { return globalData; }
    private:
        std::vector<SynthVoice*> *voices;
        std::vector<NoteData*> notes;
        int getNoteDataIndex(int channel, int note);
        int findYoungestInactiveNoteDataIndex(int channel);
        ChannelData channelData[16];
        GlobalData globalData;
    };

    class Synth {
    public:
        // TODO, encapsulate the scope stuff into it's own class so we can have several
        Synth();
        virtual ~Synth();
        VoiceBus voiceBus;
        virtual void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate);
        virtual size_t getScopeBuffer(float *buffer, size_t bufferSizeIn) const;
    private:
        std::vector<SynthVoice*> voices; // per note sound generation
        EffectChain* effects; // effects applied to mix of voices (in series)
        // settings like voice stacking, per voice detuning and etc etc goes here
        float scopeBuffer[512];
        const unsigned int scopeBufferSize;
        unsigned int scopeBufferWriteIndex;
        float scopeDrift;
    };

}
