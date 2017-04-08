#include "PhasePhckr.h"
#include "SynthVoice.hpp"
#include "VoiceBus.hpp"

namespace PhasePhckr {

void VoiceBus::handleNoteOnOff(int channel, int note, float velocity, bool on) {
    int idx = getNoteDataIndex(channel, note);
    if (on) {
        // find matching note data (or create new)
        NoteData* n;
        if (idx == -1) {
            n = new NoteData(channel, note, velocity);
            notes.push_back(n);
            idx = notes.size();
        }
        else {
            n = notes[idx];
        }

        // find a free voice state (or steal an active)
        if (n->voiceIndex == -1) {
            int i = 0;
            unsigned int oldest = 0;
            int oldestIdx = -1;
            for (const auto &v : *voices) {
                if (v->mpe.getState().gate == 0 && v->mpe.getAge() > oldest) {
                    oldest = v->mpe.getAge();
                    oldestIdx = i;
                }
                i++;
            }
            if (oldestIdx != -1) {
                n->voiceIndex = oldestIdx;
            }
            else {
                return; // TODO -- steal a voice
            }
        }
        SynthVoiceI *v = (*voices)[n->voiceIndex];
        v->mpe.on(note, velocity);
    }
    else {
        if (idx != -1) {
            NoteData* n = notes[idx];
            if (n->voiceIndex != -1) {
                SynthVoiceI *v = (*voices)[n->voiceIndex];
                v->mpe.off(note, velocity);
                // wake up the youngest but paused note (if any) on the same channel 
                int new_idx = findYoungestInactiveNoteDataIndex(channel);
                if (new_idx != -1) {
                    n = notes[new_idx];
                    v->mpe.on(n->note, n->velocity);
//                    v->mpe.press(n->notePressure); // TODO, max of note pressure and channel aftertouch?
                    n->voiceIndex = idx;
                }
            }
            delete notes[idx];
            notes.erase(notes.begin() + idx);
        }
    }
}

void VoiceBus::handleX(int channel, float position) {
    for (const auto &n : notes) {
        if (n->channel == channel) {
            if (n->voiceIndex != -1) {
                (*voices)[n->voiceIndex]->mpe.glide(position);
            }
        }
    }
}

void VoiceBus::handleY(int channel, float position) {
    for (const auto &n : notes) {
        if (n->channel == channel) {
            if (n->voiceIndex != -1) {
                (*voices)[n->voiceIndex]->mpe.slide(position);
            }
        }
    }
}

void VoiceBus::handleZ(int channel, float position) {
    for (const auto &n : notes) {
        if (n->channel == channel) {
            if (n->voiceIndex != -1) {
                (*voices)[n->voiceIndex]->mpe.press(position);
            }
        }
    }
}

void VoiceBus::handleNoteZ(int channel, int note, float position) {
    // yes, this will be design fight with handleZ if the user uses both
    // additionally by design, ignored if voice not active
    int idx = getNoteDataIndex(channel, note);
    if (idx != -1 && notes[idx]->voiceIndex != -1) {
        notes[idx]->notePressure = position;
        (*voices)[notes[idx]->voiceIndex]->mpe.press(position);
    }
}

int VoiceBus::getNoteDataIndex(int channel, int note) {
    int idx = 0;
    for (const auto &n : notes) {
        if (n->note == note && n->channel == channel) {
            return idx;
        }
        idx++;
    }
    return -1;
}

int VoiceBus::findYoungestInactiveNoteDataIndex(int channel) {
    int idx = 0;
    int min = -1;
    for (const auto &n : notes) {
        if (n->channel == channel && (int)n->age < min) {
            min = idx;
        }
        idx++;
    }
    return min;
}

void VoiceBus::update() {
    for (const auto &n : notes) {
        n->age++;
    }
}

VoiceBus::~VoiceBus() {
    for (auto &n : notes) {
        delete(n);
    }
}

}