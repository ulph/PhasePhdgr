#include "PhasePhckr.h"
#include "SynthVoice.hpp"
#include "VoiceBus.hpp"
#include <string>

namespace PhasePhckr {

void VoiceBus::handleNoteOnOff(int channel, int note, float velocity, bool on, std::vector<SynthVoice*> &voices) {
    int idx = getNoteDataIndex(channel, note);
    if (on && velocity > 0) {
        // find matching note data (or create new)
        NoteData* n;
        if (idx == -1) {
            n = new NoteData(channel, note, velocity);
            notes.push_back(n);
            idx = (int)notes.size();
        }
        else {
            n = notes[idx];
        }

        // find the oldest free voice state (or steal one)
        SynthVoice *v;
        if (n->voiceIndex == -1) {
            int i = 0;
            unsigned int oldest = 0;
            int oldestIdx = -1;
            for (const auto &v : voices) {
                if (v->mpe.getState().gate == 0 && v->mpe.getAge() > oldest) {
                    oldest = v->mpe.getAge();
                    oldestIdx = i;
                }
                i++;
            }
            if (oldestIdx == -1) {
                return; // TODO -- steal a voice ... take the oldest
            }
            voices[oldestIdx]->mpe.reset();
            n->voiceIndex = oldestIdx;
        }
        v = voices[n->voiceIndex];
        v->mpe.on(note, velocity);
        v->mpe.glide(channelData[channel].x);
        v->mpe.slide(channelData[channel].y);
        v->mpe.press(channelData[channel].z);
    }
    else {
        if (idx != -1) {
            NoteData* n = notes[idx];
            if (n->voiceIndex != -1) {
                SynthVoice *v = voices[n->voiceIndex];
                v->mpe.off(note, velocity);
                // TODO -- wake up a note ... take the youngest
            }
            delete notes[idx];
            notes.erase(notes.begin() + idx);
        }
    }
}

void VoiceBus::handleX(int channel, float position, std::vector<SynthVoice*> &voices) {
    channelData[channel].x = position;
    for (const auto &n : notes) {
        if (n->channel == channel) {
            if (n->voiceIndex != -1) {
                voices[n->voiceIndex]->mpe.glide(position);
            }
        }
    }
}

void VoiceBus::handleY(int channel, float position, std::vector<SynthVoice*> &voices) {
    channelData[channel].y = position;
    for (const auto &n : notes) {
        if (n->channel == channel) {
            if (n->voiceIndex != -1) {
                voices[n->voiceIndex]->mpe.slide(position);
            }
        }
    }
}

void VoiceBus::handleZ(int channel, float position, std::vector<SynthVoice*> &voices) {
    channelData[channel].z = position;
    for (const auto &n : notes) {
        if (n->channel == channel) {
            if (n->voiceIndex != -1) {
                voices[n->voiceIndex]->mpe.press(position);
            }
        }
    }
}

void VoiceBus::handleNoteZ(int channel, int note, float position, std::vector<SynthVoice*> &voices) {
    // yes, this will be design fight with handleZ if the user uses both
    int idx = getNoteDataIndex(channel, note);
    if (idx != -1) {
        notes[idx]->notePressure = position;
        if (notes[idx]->voiceIndex != -1) {
            voices[notes[idx]->voiceIndex]->mpe.press(position);
        }
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

float VoiceBus::findScopeVoiceHz(std::vector<SynthVoice*> &voices) {
    unsigned int max = 0;
    for (const auto &v : voices) {
        if(v->mpe.getState().gate && v->mpe.getAge() > max){
            scopeHz = v->mpe.getState().pitchHz;
        }
    }
    return scopeHz;
}

void VoiceBus::update() {
    // only relative age matter so per chunk is ok
    for (const auto &n : notes) {
        n->age++;
    }
}

VoiceBus::VoiceBus() : scopeHz(0)
{
}

VoiceBus::~VoiceBus() {
    for (auto &n : notes) {
        delete(n);
    }
}

}
