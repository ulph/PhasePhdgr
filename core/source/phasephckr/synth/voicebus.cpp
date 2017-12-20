#include <string>

#include "voicebus.hpp"
#include "synthvoice.hpp"

namespace PhasePhckr {

void VoiceBus::handleNoteOnOff(int channel, int note, float velocity, bool on, std::vector<SynthVoice*> &voices) {
    int idx = getNoteDataIndex(channel, note);
    if (on && velocity > 0) {
        bool newVoice = true;

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

        // find a free voice - 1. oldest inactive silent, 2. oldest inactive, 3. depending on stealPolicy, take active voice
        SynthVoice *freeVoice;
        if (n->voiceIndex == -1) {
            unsigned int oldestInactiveSilent = 0;
            int selectedInactiveSilentIdx = -1;

            unsigned int oldestInactive = 0;
            int selectedInactiveIdx = -1;

            unsigned int oldestActive = 0;
            unsigned int lowestActive = UINT_MAX;
            unsigned int highestActive = 0;
            float quietestActive = 99.f;
            int selectedActiveIdx = -1;

            int i = 0;
            for (const auto *v : voices) {
                auto age = v->mpe.getAge();
                if (v->mpe.getState().gateTarget == 0) {
                    // active a voice
                    if (v->isSilent() && age > oldestInactiveSilent) {
                        oldestInactiveSilent = age;
                        selectedInactiveSilentIdx = i;
                    }
                    else if (age > oldestInactive) {
                        oldestInactive = age;
                        selectedInactiveIdx = i;
                    }
                }
                else if(stealPolicy != NoteStealPolicyDoNotSteal){
                    // steal a voice
                    switch (stealPolicy) {
                    case NoteStealPolicyStealOldest:
                        if (age > oldestActive) {
                            oldestActive = age;
                            selectedActiveIdx = i;
                        }
                        break;
                    case NoteStealPolicyStealLowestRMS:
                        if (v->getRms() < quietestActive) {
                            quietestActive = v->getRms();
                            selectedActiveIdx = i;
                        }
                        break;
                    case NoteStealPolicyDoNotSteal:
                    default:
                        break;
                    }
                }
                i++;
            }

            int idxToUse = -1;
            if (selectedInactiveSilentIdx != -1) {
                idxToUse = selectedInactiveSilentIdx;
            }
            else if (selectedInactiveIdx != -1) {
                idxToUse = selectedInactiveIdx;
            }
            else if (selectedActiveIdx != -1) {
                idxToUse = selectedActiveIdx;
                newVoice = false;
            }

            if(idxToUse < 0 || idxToUse >= voices.size()){
                return; 
            }

            voices[idxToUse]->mpe.reset();
            n->voiceIndex = idxToUse;
        }
        else {
            // TODO, corner case when existing voice ...
            // should that retrigger or be legato??
            // probably retrigger
        }

        n->velocity = newVoice ? velocity : legato ? n->velocity : velocity;
        n->state = NoteState::ON;

        freeVoice = voices[n->voiceIndex];
        freeVoice->mpe.on(note, n->velocity, newVoice ? false : legato);
        freeVoice->mpe.glide(channelData[channel].x);
        freeVoice->mpe.slide(channelData[channel].y);
        freeVoice->mpe.press(channelData[channel].z);

    }
    else if (idx != -1) {
        if (channelData[channel].sustain < 0.5f) {
            if (notes.at(idx).voiceIndex != -1)
                voices[notes.at(idx).voiceIndex]->mpe.off(note, velocity);
            notes.erase(notes.begin() + idx);
        }
        else {
            notes.at(idx).velocity = 0.0f;
            notes.at(idx).state = NoteState::SUSTAINED;
        }
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

void VoiceBus::handleSustain(int channel, float position, std::vector<SynthVoice*> &voices) {
    if (channelData[channel].sustain >= 0.5f && position < 0.5f) {
        for (auto it = notes.begin(); it != notes.end();) {
            if (it->channel == channel && it->state == NoteState::SUSTAINED) {
                if (it->voiceIndex != -1) {
                    voices[it->voiceIndex]->mpe.off(it->note, it->velocity);
                }
                it = notes.erase(it);
            }
            else {
                it++;
            }
        }
    }
    channelData[channel].sustain = position;
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
        if(v->mpe.getState().gateTarget && v->mpe.getAge() > max){
            idx = i;
            max = v->mpe.getAge();
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
