#include <string>

#include "voicebus.hpp"
#include "synthvoice.hpp"

namespace PhasePhckr {

void VoiceBus::handleNoteOnOff(int channel, int note, float velocity, bool on, std::vector<SynthVoice*> &voices) {
    int idx = getNoteDataIndex(channel, note);
    if (on && velocity > 0) {
        // find matching note data (or create new)
        NoteData* n = nullptr;
        if (idx == -1) {
            notes.emplace_back(channel, note, velocity);
            n = &notes.back();
            idx = (int)notes.size();
        }
        else {
            n = &notes[idx];
        }

        // find a free voice - 1. oldest inactive silent, 2. oldest inactive, 3. oldest ...
        SynthVoice *freeVoice;
        if (n->voiceIndex == -1) {
            unsigned int oldestInactiveSilent = 0;
            int oldestInactiveSilentIdx = -1;

            unsigned int oldestInactive = 0;
            int oldestInactiveIdx = -1;

            unsigned int oldest = 0;
            int oldestIdx = -1;

            int i = 0;
            for (const auto *v : voices) {
                auto age = v->mpe.getAge();
                if (v->mpe.getState().gate == 0) {
                    if (v->isSilent() && age > oldestInactiveSilent) {
                        oldestInactiveSilent = age;
                        oldestInactiveSilentIdx = i;
                    }
                    else if (age > oldestInactive) {
                        oldestInactive = age;
                        oldestInactiveIdx = i;
                    }
                }
                else if (age > oldest){
                    oldest = age;
                    oldestIdx = i;
                }
                i++;
            }

            int idxToUse = -1;
            if (oldestInactiveSilentIdx != -1) {
                idxToUse = oldestInactiveSilentIdx;
            }
            else if (oldestInactive == -1) {
                idxToUse = oldestInactive;
            }
            else if (oldestIdx != -1) {
                idxToUse = oldestIdx; // TODO, want this?
            }
            else {
                assert(0); return; 
            }

            voices[idxToUse]->mpe.reset();
            n->voiceIndex = idxToUse;
        }

        freeVoice = voices[n->voiceIndex];
        freeVoice->mpe.on(note, velocity);
        freeVoice->mpe.glide(channelData[channel].x);
        freeVoice->mpe.slide(channelData[channel].y);
        freeVoice->mpe.press(channelData[channel].z);
    }
    else if (idx != -1) {
        if (notes.at(idx).voiceIndex != -1) 
            voices[notes.at(idx).voiceIndex]->mpe.off(note, velocity);
        notes.erase(notes.begin() + idx);
    }
}

void VoiceBus::handleX(int channel, float position, std::vector<SynthVoice*> &voices) {
    channelData[channel].x = position;
    for (const auto &n : notes) {
        if (n.channel == channel) {
            if (n.voiceIndex != -1) {
                voices[n.voiceIndex]->mpe.glide(position);
            }
        }
    }
}

void VoiceBus::handleY(int channel, float position, std::vector<SynthVoice*> &voices) {
    channelData[channel].y = position;
    for (const auto &n : notes) {
        if (n.channel == channel) {
            if (n.voiceIndex != -1) {
                voices[n.voiceIndex]->mpe.slide(position);
            }
        }
    }
}

void VoiceBus::handleZ(int channel, float position, std::vector<SynthVoice*> &voices) {
    channelData[channel].z = position;
    for (const auto &n : notes) {
        if (n.channel == channel) {
            if (n.voiceIndex != -1) {
                voices[n.voiceIndex]->mpe.press(position);
            }
        }
    }
}

void VoiceBus::handleNoteZ(int channel, int note, float position, std::vector<SynthVoice*> &voices) {
    int idx = getNoteDataIndex(channel, note);
    if (idx != -1) {
        notes[idx].notePressure = position;
        if (notes[idx].voiceIndex != -1) {
            voices[notes[idx].voiceIndex]->mpe.press(position);
        }
    }
}

int VoiceBus::getNoteDataIndex(int channel, int note) {
    int idx = 0;
    for (const auto &n : notes) {
        if (n.note == note && n.channel == channel) {
            return idx;
        }
        idx++;
    }
    return -1;
}

int VoiceBus::findScopeVoiceIndex(std::vector<SynthVoice*> &voices) {
    unsigned int max = 0;
    int idx = -1;
    int i = 0;
    for (const auto &v : voices) {
        if(v->mpe.getState().gate && v->mpe.getAge() > max){
            idx = i;
        }
        i++;
    }
    return idx;
}

void VoiceBus::update() {
    for (auto &n : notes) {
        n.age++;
    }
}

}
