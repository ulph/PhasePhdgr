#include "PhasePhckr.h"
#include "SynthVoice.hpp"

namespace PhasePhckr {

void VoiceBus::handleNoteOnOff(int channel, int note, float velocity, bool on)
{
    SynthVoiceI *v = (*voices)[channel];
    on ? v->mpe.on(note, velocity) : v->mpe.off(note, velocity);
}

void VoiceBus::handleX(int channel, float position) {
    (*voices)[channel]->mpe.glide(position);
}

void VoiceBus::handleY(int channel, float position) {
    (*voices)[channel]->mpe.slide(position);
}

void VoiceBus::handleZ(int channel, float position) {
    (*voices)[channel]->mpe.press(position);
}

}