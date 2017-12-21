#pragma once

#include "synthvoice.hpp"

#include <stack>
#include <vector>

namespace PhasePhckr {

enum class NoteState {
    ON,
    SUSTAINED,
    STOLEN
};

struct StolenNoteData {
    int channel;
    int note;
    StolenNoteData(int c, int n) 
        : channel(c)
        , note(n)
    {}
};

struct NoteData {
    NoteState state;
    int channel;
    int note;
    int voiceIndex;
    float notePressure;
    float velocity;
    NoteData(int channel, int note, float velocity) 
        : state(NoteState::ON)
        , channel(channel)
        , note(note)
        , voiceIndex(-1)
        , notePressure(0)
        , velocity(velocity)
    {
    }
};

struct ChannelData {
    ChannelData() 
        : x(0.0f)
        , y(0.0f)
        , z(0.0f)
        , sustain(0.0f) 
    {}
    float x;
    float y;
    float z;
    float sustain;
};

class VoiceBus {
public:
    void handleNoteOnOff(int channel, int note, float velocity, bool on, std::vector<SynthVoice*> &voices);
    void handleX(int channel, float position, std::vector<SynthVoice*> &voices);
    void handleY(int channel, float position, std::vector<SynthVoice*> &voices);
    void handleZ(int channel, float position, std::vector<SynthVoice*> &voices);
    void handleNoteZ(int channel, int note, float position, std::vector<SynthVoice*> &voices);
    void handleSustain(int channel, float position, std::vector<SynthVoice*> &voices);
    void update();
    int findScopeVoiceIndex(std::vector<SynthVoice*> &voices);
    void setLegato(bool status) { legato = status; }
    void setStealPolicy(NoteStealPolicy newPolicy) { stealPolicy = newPolicy; }
    void setActivationPolicy(NoteActivationPolicy newPolicy) { activationPolicy = newPolicy; }
private:
    enum class fvr {
        NewVoice,
        StolenVoice,
        NoVoice,
        WaitingForVoice
    };
    fvr findVoice(int note, int& idxToUse, const std::vector<SynthVoice*> &voices);
    void handleNoteOn(int channel, int note, float velocity, std::vector<SynthVoice*> &voices);
    void handleNoteOff(int channel, int note, float velocity, std::vector<SynthVoice*> &voices);
    NoteStealPolicy stealPolicy = NoteStealPolicyDoNotSteal;
    NoteActivationPolicy activationPolicy = NoteActivationPolicyPreferOldestSilent;
    bool legato = false;
    std::vector<NoteData> notes; // make map of channel,note instead ??
    std::vector<StolenNoteData> stolenNotes;
    int getNoteDataIndex(int channel, int note);
    int getNoteDataIndexForStealingVoice(int voiceIdx);
    ChannelData channelData[16];
};

}
