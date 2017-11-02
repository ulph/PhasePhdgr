#pragma once

#include "synthvoice.hpp"

namespace PhasePhckr {

struct NoteData {
    int channel;
    int note;
    float velocity; // off velocity is meaningless as it's a transient state
    int voiceIndex;
    float notePressure;
    unsigned int age;
    NoteData(int channel, int note, float velocity) :
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

class VoiceBus {
public:
    VoiceBus();
    virtual ~VoiceBus();
    void handleNoteOnOff(int channel, int note, float velocity, bool on, std::vector<SynthVoice*> &voices);
    void handleX(int channel, float position, std::vector<SynthVoice*> &voices);
    void handleY(int channel, float position, std::vector<SynthVoice*> &voices);
    void handleZ(int channel, float position, std::vector<SynthVoice*> &voices);
    void handleNoteZ(int channel, int note, float position, std::vector<SynthVoice*> &voices);
    void update();
    int findScopeVoiceIndex(std::vector<SynthVoice*> &voices);
private:
    std::vector<NoteData*> notes;
    int getNoteDataIndex(int channel, int note);
    ChannelData *channelData;
};

}
