#include <string>

#include "voicebus.hpp"
#include "synthvoice.hpp"

#include <limits>

namespace PhasePhckr {

 VoiceBus::fvr VoiceBus::findVoice(int newNote, NoteData* noteData, const std::vector<SynthVoice*> &voices) {
    // active or steal a voice based on policies

    fvr res = fvr::NoVoice;
    noteData->voiceIndex = -1;

    unsigned int oldestInactiveSilent = 0;
    auto selectedInactiveSilentIdx = -1;

    unsigned int oldestInactive = 0;
    unsigned int youngestInactive = UINT_MAX;
    auto selectedInactiveIdx = -1;

    auto youngestActive = UINT_MAX;
    auto closestDistanceActive = INT_MAX;
    auto lowestActive = INT_MAX;
    auto highestActive = INT_MIN;
    auto quietestActive = std::numeric_limits<float>::max();
    auto selectedActiveIdx = -1;

    float stolenNoteVelocity = 0;

    int i = 0;
    for (const auto *v : voices) {
        auto age = v->mpe.getAge();
        auto note = v->mpe.getRootNote();
        auto rms = v->getRms();
        auto vel = v->mpe.getState().strikeZ;
        auto distance = abs(newNote-note);

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
                case NoteActivationPolicyOldest:
                    if (age > oldestInactive) {
                        oldestInactive = age;
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
            if (stealPolicy != NoteStealPolicyNone && stealPolicy != NoteStealPolicyOldest) {
                // steal a voice based on policy
                switch (stealPolicy) {
                case NoteStealPolicyIfLower:
                    if (note < lowestActive) {
                        lowestActive = note;
                        selectedActiveIdx = i;
                        stolenNoteVelocity = vel;
                    }
                    break;
                case NoteStealPolicyIfHigher:
                    if (note > highestActive) {
                        highestActive = note;
                        selectedActiveIdx = i;
                        stolenNoteVelocity = vel;
                    }
                    break;
                case NoteStealPolicyLowestRMS:
                    if (rms < quietestActive) {
                        quietestActive = rms;
                        selectedActiveIdx = i;
                        stolenNoteVelocity = vel;
                    }
                    break;
                case NoteStealPolicyYoungest:
                    if (age < youngestActive) {
                        youngestActive = age;
                        selectedActiveIdx = i;
                        stolenNoteVelocity = vel;
                    }
                    break;
                case NoteStealPolicyClosest:
                    if (distance < closestDistanceActive) {
                        closestDistanceActive = distance;
                        selectedActiveIdx = i;
                        stolenNoteVelocity = vel;
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

    if(selectedInactiveSilentIdx == -1 && selectedInactiveIdx == -1 && stealPolicy == NoteStealPolicyOldest){
        // a bit special, as mpe state has lost knowledge of when a key was originally pressed down
        for(int i=0; i<notes.size(); i++){
            const auto& n = notes.at(i);
            if(n.voiceIndex >= 0 && n.state == NoteState::ON && n.voiceIndex < voices.size()){
                selectedActiveIdx = n.voiceIndex;
                break;
            }
        }
    }

    if (selectedInactiveSilentIdx != -1) {
        noteData->voiceIndex = selectedInactiveSilentIdx;
        res = fvr::NewVoice;
    }
    else if (selectedInactiveIdx != -1) {
        noteData->voiceIndex = selectedInactiveIdx;
        res = fvr::NewVoice;
    }
    else if (selectedActiveIdx != -1) {
        if (
            (stealPolicy == NoteStealPolicyIfLower && newNote < lowestActive)
        ||  
            (stealPolicy == NoteStealPolicyIfHigher && newNote > highestActive)
        ||
            (stealPolicy == NoteStealPolicyLowestRMS)
        ||
            (stealPolicy == NoteStealPolicyOldest)
        ||
            (stealPolicy == NoteStealPolicyYoungest)
        ||
            (stealPolicy == NoteStealPolicyClosest)
        // ...
        ) {
            auto nIdxToSteal = getNoteDataIndexForStealingVoice(selectedActiveIdx);
            if (nIdxToSteal != -1) {
                auto& nts = notes[nIdxToSteal];
                nts.state = NoteState::STOLEN;
                noteData->voiceIndex = selectedActiveIdx;
                noteData->velocity = stolenNoteVelocity; // in case of legato
                res = fvr::StolenVoice;
            }
            else assert(0);
        }
        else if (stealPolicy != NoteStealPolicyNone) { // TODO refactor
            noteData->state = NoteState::STOLEN;
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
    else n = &notes[idx];

    // prevent re-activation of stolen voices
    if(n->state == NoteState::STOLEN) return;

    // find a free voice
    fvr r;
    if(n->voiceIndex == -1) r = findVoice(note, n, voices);
    else r = fvr::StolenVoice;

    // not possible to assign a voice nor wait for one, throw the note state away and bail out
    if (r == fvr::NoVoice) {
        notes.erase(notes.begin() + idx);
        return;
    }

    // update channel/note
    n->channel = channel;
    n->note = note;

    // update velocity
    if(r != fvr::StolenVoice) n->velocity = velocity;
    else if( legato == LegatoModeFreezeVelocity ) n->velocity = n->velocity;
    else n->velocity = velocity;

    // if waiting for voice, or error condition, bail out
    if (r == fvr::WaitingForVoice) return;
    if (n->voiceIndex < 0 || n->voiceIndex >= voices.size()) return;

    // reset the voice
    if (fvr::StolenVoice == r && !legato) voices[n->voiceIndex]->mpe.reset(true);
    else if(fvr::NewVoice == r) voices[n->voiceIndex]->mpe.reset(false);

    // trigger the voice
    n->state = NoteState::ON;
    SynthVoice *selectedVoice = voices[n->voiceIndex];
    selectedVoice->mpe.on(note, n->velocity, fvr::NewVoice == r ? false : legato != LegatoModeRetrigger);
    selectedVoice->mpe.glide(channelData[channel].x);
    selectedVoice->mpe.slide(channelData[channel].y);
    selectedVoice->mpe.press(channelData[channel].z);
    if(fvr::NewVoice == r) selectedVoice->mpe.fillGlideSlidePress();
}

void VoiceBus::handleNoteOff(int channel, int note, float velocity, std::vector<SynthVoice*> &voices) {
    int idx = getNoteDataIndex(channel, note);
    if (idx == -1) return;

    if (channelData[channel].sustain < 0.5f) {
        if(notes.at(idx).state == NoteState::ON) {
            int voiceIdx = notes.at(idx).voiceIndex;
            if (voiceIdx != -1 && voiceIdx < voices.size()) {
                if(voices.at(voiceIdx)->mpe.getState().gateTarget > 0) {
                    auto closestDistance = INT_MAX;
                    auto toReviveIdx = -1;
                    auto lowestWaiting = INT_MAX;
                    auto highestWaiting = INT_MIN;

                    if (reactivationPolicy != NoteReactivationPolicyNone) {
                        // sift through the stolen notes and revive the one matching the policy
                        for (auto sIdx = 0; sIdx < notes.size(); sIdx++) {
                            if (notes.at(sIdx).state == NoteState::STOLEN) {
                                if (notes.at(sIdx).note == note && notes.at(sIdx).channel == channel) {
                                    continue;
                                }
                                else {
                                    auto sNote = notes.at(sIdx).note;
                                    auto distance = abs(note - sNote);
                                    if (reactivationPolicy == NoteReactivationPolicyHighest && sNote > highestWaiting) {
                                        highestWaiting = sNote;
                                        toReviveIdx = sIdx;
                                    }
                                    else if (reactivationPolicy == NoteReactivationPolicyLowest && sNote < lowestWaiting) {
                                        lowestWaiting = sNote;
                                        toReviveIdx = sIdx;
                                    }
                                    else if (reactivationPolicy == NoteReactivationPolicyLast) {
                                        toReviveIdx = sIdx;
                                    }
                                    else if (reactivationPolicy == NoteReactivationPolicyFirst && toReviveIdx == -1) {
                                        toReviveIdx = sIdx;
                                    }
                                    else if (reactivationPolicy == NoteReactivationPolicyClosest && distance < closestDistance) {
                                        closestDistance = distance;
                                        toReviveIdx = sIdx;
                                    }
                                    // ...
                                }
                            }
                        }
                    }

                    if (toReviveIdx != -1) {
                        float legatoVelocity = voices[voiceIdx]->mpe.getState().strikeZ;
                        auto& toRevive = notes.at(toReviveIdx);
                        toRevive.voiceIndex = voiceIdx;
                        bool doLegato = legato != LegatoModeRetrigger && legato != LegatoModeRetriggerReleaseVelocity;
                        if(legato == LegatoModeFreezeVelocity)
                            toRevive.velocity = legatoVelocity;
                        else if(legato == LegatoModeReleaseVelocity)
                            toRevive.velocity = velocity;
                        else if (legato == LegatoModeRetriggerReleaseVelocity)
                            toRevive.velocity = velocity;
                        // ...
                        toRevive.state = NoteState::ON;
                        voices[voiceIdx]->mpe.on(toRevive.note, toRevive.velocity, doLegato);
                    }
                    else {
                        voices[voiceIdx]->mpe.off(note, velocity);
                    }
                }
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
    // TODO, sanitize/clean up (at least in debug builds)
}

}
