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
    {};
};

}