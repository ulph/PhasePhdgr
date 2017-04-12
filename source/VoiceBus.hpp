#pragma once

#include "MPEVoice.hpp"

namespace PhasePhckr {

struct NoteData {
    int channel;
    int note;
    float velocity; // off velocity is meaningless as it's a transient state
    int voiceIndex;
    float notePressure;
    unsigned int age;
    NoteData(int channel, int note, float velocity):
        channel(channel), 
        note(note), 
        velocity(velocity), 
        voiceIndex(-1),
        notePressure(0),
        age(0)
    {}
};

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

}
