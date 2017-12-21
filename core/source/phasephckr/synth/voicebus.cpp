#include <string>

#include "voicebus.hpp"
#include "synthvoice.hpp"

#include <limits>

namespace PhasePhckr {

 VoiceBus::fvr VoiceBus::findVoice(int newNote, int& voiceIdxToUse, const std::vector<SynthVoice*> &voices) {
    // active or steal a voice based on policies

    fvr res = fvr::NoVoice;
    voiceIdxToUse = -1;

    unsigned int oldestInactiveSilent = 0;
    auto selectedInactiveSilentIdx = -1;

    unsigned int oldestInactive = 0;
    unsigned int youngestInactive = UINT_MAX;
    auto selectedInactiveIdx = -1;

    unsigned int oldestActive = 0;
    auto lowestActive = INT_MAX;
    auto highestActive = INT_MIN;
    auto quietestActive = std::numeric_limits<float>::max();
    auto selectedActiveIdx = -1;

    int i = 0;
    for (const auto *v : voices) {
        auto age = v->mpe.getAge();
        auto note = v->mpe.getRootNote();
        auto rms = v->getRms();

        if (v->mpe.getState().gateTarget == 0) {
            if (v->isSilent()) {
                // always find the oldest inactive silent
                if (age > oldestInactiveSilent) {
                    oldestInactiveSilent = age;
                    selectedInactiveSilentIdx = i;
                }
            }
            else if (activationPolicy != NoteActivationPolicyOnlySilent){
                // find an inactive (non-silent) based on policy
                switch (activationPolicy) {
                case NoteActivationPolicyPreferOldestSilent:
                case NoteActivationPolicyPreferOldestNotSilent:
                    if (age > oldestInactive) {
                        oldestInactive = age;
                        selectedInactiveIdx = i;
                    }
                    break;
                case NoteActivationPolicyPreferYoungestNotSilent:
                    if (age < youngestInactive) {
                        youngestInactive = age;
                        selectedInactiveIdx = i;
                    }
                    break;
                default:
                    PP_NYI;
                    break;
                }
            }
        }
        else {
            if (stealPolicy != NoteStealPolicyDoNotSteal) {
                // steal a voice based on policy
                switch (stealPolicy) {
                case NoteStealPolicyStealIfLower:
                    if (note < lowestActive) {
                        lowestActive = note;
                        selectedActiveIdx = i;
                    }
                    break;
                case NoteStealPolicyStealIfHigher:
                    if (note > highestActive) {
                        highestActive = note;
                        selectedActiveIdx = i;
                    }
                    break;
                case NoteStealPolicyStealOldest:
                    if (age > oldestActive) {
                        oldestActive = age;
                        selectedActiveIdx = i;
                    }
                    break;
                case NoteStealPolicyStealLowestRMS:
                    if (rms < quietestActive) {
                        quietestActive = rms;
                        selectedActiveIdx = i;
                    }
                    break;
                default:
                    PP_NYI;
                    break;
                }
            }
        }
        i++;
    }

    if (selectedInactiveSilentIdx != -1) {
        voiceIdxToUse = selectedInactiveSilentIdx;
        res = fvr::NewVoice;
    }
    else if (selectedInactiveIdx != -1) {
        voiceIdxToUse = selectedInactiveIdx;
        res = fvr::NewVoice;
    }
    else if (selectedActiveIdx != -1) {
        if (
            (stealPolicy == NoteStealPolicyStealIfLower && newNote < lowestActive)
        ||  
            (stealPolicy == NoteStealPolicyStealIfHigher && newNote > highestActive)
        ) {
            auto nIdxToSteal = getNoteDataIndexForStealingVoice(selectedActiveIdx);
            if (nIdxToSteal != -1) {
                auto& nts = notes[nIdxToSteal];
                nts.state = NoteState::STOLEN;
                stolenNotes.emplace_back(nts.channel, nts.note);
                voiceIdxToUse = selectedActiveIdx;
                res = fvr::StolenVoice;
            }
            else {
                assert(0); // eh, this should not happen!
            }
        }
        else if (stealPolicy == NoteStealPolicyStealIfLower || stealPolicy == NoteStealPolicyStealIfHigher) {
            res = fvr::WaitingForVoice;
        }
    }

    return res;
}

void VoiceBus::handleNoteOn(int channel, int note, float velocity, std::vector<SynthVoice*> &voices) {
    int idx = getNoteDataIndex(channel, note);

    // find matching note data (or create new)
    NoteData* n = nullptr;
    if (idx == -1) {
        notes.emplace_back(channel, note, velocity);
        n = &notes.back();
        idx = (int)notes.size()-1;
    }
    else {
        n = &notes[idx];
    }

    fvr r = fvr::StolenVoice;

    // find a free voice (if note has none assigned)

    if (n->voiceIndex == -1) {
        r = findVoice(note, n->voiceIndex, voices);
    }
    else {
        r = fvr::StolenVoice; // from itself, so to speak
    }

    // act on the results, if NoVoice or WaitingForVoice, bail out

    if (r == fvr::NoVoice) {
        notes.erase(notes.begin() + idx);
        return;
    }

    n->channel = channel;
    n->note = note;
    n->velocity = fvr::StolenVoice == r ? velocity : legato ? n->velocity : velocity;

    if (r == fvr::WaitingForVoice) {
        n->state = NoteState::STOLEN;
        stolenNotes.emplace_back(channel, note);
        return;
    }

    if (n->voiceIndex < 0 || n->voiceIndex >= voices.size()) return;

    // ok, all good. let's see of legato or not

    if (
        fvr::StolenVoice == r && !legato
        ||
        fvr::NewVoice == r
    ) {
        voices[n->voiceIndex]->mpe.reset();
        // TODO, fill in the mpe-esque things which actually could live in channelData
    }

    // trigger the voice

    n->state = NoteState::ON;

    SynthVoice *selectedVoice = voices[n->voiceIndex];
    selectedVoice->mpe.on(note, n->velocity, fvr::NewVoice == r ? false : legato);
    selectedVoice->mpe.glide(channelData[channel].x);
    selectedVoice->mpe.slide(channelData[channel].y);
    selectedVoice->mpe.press(channelData[channel].z);
}

void VoiceBus::handleNoteOff(int channel, int note, float velocity, std::vector<SynthVoice*> &voices) {
    int idx = getNoteDataIndex(channel, note);
    if (idx == -1) return;

    if (channelData[channel].sustain < 0.5f) {
        bool shouldEraseNote = true;
        int voiceIdx = notes.at(idx).voiceIndex;
        float legatoVelocity = notes.at(idx).velocity;

        if (voiceIdx != -1) {
            auto toReviveIdx = -1;
            auto policyScanIdx = -1;

            auto lowestWaiting = INT_MAX;
            auto highestWaiting = INT_MIN;

            // sift through the stolen notes and revive the one matching the policy
            for (auto sIt = stolenNotes.begin(); sIt != stolenNotes.end();) {
                auto sIdx = getNoteDataIndex(sIt->channel, sIt->note);
                if (sIdx != -1 && notes.at(sIdx).note == note && notes.at(sIdx).channel == channel) {
                    sIt = stolenNotes.erase(sIt);
                }
                else if (sIdx != -1 && (stealPolicy == NoteStealPolicyStealIfHigher || stealPolicy == NoteStealPolicyStealIfLower)) {
                    auto sNote = notes.at(sIdx).note;
                    if (stealPolicy == NoteStealPolicyStealIfHigher && sNote > highestWaiting) {
                        highestWaiting = sNote;
                        policyScanIdx = sIdx;
                    }
                    else if(stealPolicy == NoteStealPolicyStealIfLower && sNote < lowestWaiting) {
                        lowestWaiting = sNote;
                        policyScanIdx = sIdx;
                    }
                    sIt++;
                }
                else {
                    sIt = stolenNotes.erase(sIt);
                }
            }

            if (stealPolicy == NoteStealPolicyStealIfHigher && highestWaiting < note) {
                toReviveIdx = policyScanIdx;
            }
            else if(stealPolicy == NoteStealPolicyStealIfLower && lowestWaiting > note) {
                toReviveIdx = policyScanIdx;
            }

            if (toReviveIdx != -1) {
                auto& toRevive = notes.at(toReviveIdx);
                toRevive.voiceIndex = voiceIdx;
                toRevive.velocity = legato ? legatoVelocity : toRevive.velocity;
                toRevive.state = NoteState::ON;
                voices[voiceIdx]->mpe.on(toRevive.note, toRevive.velocity, legato);
            }
            else {
                voices[voiceIdx]->mpe.off(note, velocity);
            }
        }

        notes.erase(notes.begin() + idx);

    }
    else {
        auto& n = notes.at(idx);
        n.velocity = 0.0f;
        n.state = NoteState::SUSTAINED;
    }
}

void VoiceBus::handleNoteOnOff(int channel, int note, float velocity, bool on, std::vector<SynthVoice*> &voices) {
    if (on && velocity > 0) {
        handleNoteOn(channel, note, velocity, voices);
    }
    else{
        handleNoteOff(channel, note, velocity, voices);
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

int VoiceBus::getNoteDataIndexForStealingVoice(int voiceIdx) {
    int idx = 0;
    for (const auto &n : notes) {
        if (n.voiceIndex == voiceIdx && n.state != NoteState::STOLEN) {
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
    // TODO, clean up notes? 
    // there's no guarantee we won't leak here with the different policies etc in play
    // also, make sure only one note is assigned ON/SUSTAIN per voice
    // also, make sure there's only one note of given number per channel
}

}
