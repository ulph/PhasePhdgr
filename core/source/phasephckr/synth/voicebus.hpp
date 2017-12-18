#pragma once

#include "synthvoice.hpp"

namespace PhasePhckr {

enum class NoteState {
//    OFF,
    ON,
    SUSTAINED
};

struct NoteData {
    NoteState state;
    int channel;
    int note;
    int voiceIndex;
    float notePressure;
    float velocity;
    unsigned int age;
    NoteData(int channel, int note, float velocity) 
        : state(NoteState::ON)
        , channel(channel)
        , note(note)
        , voiceIndex(-1)
        , notePressure(0)
        , velocity(velocity)
        , age(0)
    {
    }
};

struct ChannelData {
    ChannelData() : x(0), y(0), z(0) {}
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
private:
    std::vector<NoteData> notes;
    int getNoteDataIndex(int channel, int note);
    ChannelData channelData[16];
};

}
