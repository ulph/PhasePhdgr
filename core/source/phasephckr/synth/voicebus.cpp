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

    int i = 0;
    for (const auto *v : voices) {
        auto age = v->mpe.getAge();
        auto note = v->mpe.getRootNote();
        auto rms = v->getRms();
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
                    }
                    break;
                case NoteStealPolicyIfHigher:
                    if (note > highestActive) {
                        highestActive = note;
                        selectedActiveIdx = i;
                    }
                    break;
                case NoteStealPolicyLowestRMS:
                    if (rms < quietestActive) {
                        quietestActive = rms;
                        selectedActiveIdx = i;
                    }
                    break;
                case NoteStealPolicyYoungest:
                    if (age < youngestActive) {
                        youngestActive = age;
                        selectedActiveIdx = i;
                    }
                    break;
                case NoteStealPolicyClosest:
                    if (distance < closestDistanceActive) {
                        closestDistanceActive = distance;
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

    if(selectedInactiveSilentIdx == -1 && selectedInactiveIdx == -1 && stealPolicy == NoteStealPolicyOldest){
        // a bit special, as mpe state has lost knowledge of when a key was originally pressed down
        for(int i=0; i<notes.size(); i++){
            const auto& n = notes.at(i);
            if(n.voiceIndex >= 0 && !(n.state & NoteState::STOLEN) && n.voiceIndex < voices.size()){
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
                nts.state = (NoteState) (nts.state | NoteState::STOLEN);
                noteData->voiceIndex = selectedActiveIdx;
                noteData->velocity = nts.velocity; // in case of legato
                res = fvr::StolenVoice;
            }
            else assert(0);
        }
        else if (stealPolicy != NoteStealPolicyNone) { // TODO refactor
            noteData->state = (NoteState)(noteData->state | NoteState::STOLEN);
            res = fvr::WaitingForVoice;
        }
    }

    return res;
}

void VoiceBus::handleNoteOn(int channel, int note, float velocity, std::vector<SynthVoice*> &voices) {
    int idx = getNoteDataIndex(channel, note);

    // special handling for retriggering sustained notes
    if (idx != -1) {
        auto n = &notes[idx];
        if (n->voiceIndex >= 0 && n->voiceIndex < voices.size()) {
            if ((n->state == NoteState::SUSTAINED) && legato == LegatoModeRetrigger) {
                voices[n->voiceIndex]->mpe.off(note, 0.0f);
                // TODO, should not be needed to delete it
                notes.erase(notes.begin() + idx);
                assert(getNoteDataIndex(channel, note) == -1);
                idx = -1;
            }
        }
    }

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
    if (n->voiceIndex == -1) {
        r = findVoice(note, n, voices);
    }
    else if (n->state == NoteState::SUSTAINED_AND_STOLEN) {
        r = findVoice(note, n, voices);
    }
    else { 
        r = fvr::StolenVoice;
    }

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

    if (sustain < 0.5f) {
        if(notes.at(idx).state == NoteState::ON) {
            int voiceIdx = notes.at(idx).voiceIndex;
            if (voiceIdx != -1 && voiceIdx < voices.size()) {
                voices[voiceIdx]->mpe.off(note, velocity);

                auto closestDistance = INT_MAX;
                auto toReviveIdx = -1;
                auto lowestWaiting = INT_MAX;
                auto highestWaiting = INT_MIN;

                if (reactivationPolicy != NoteReactivationPolicyNone) {
                    // sift through the stolen notes and revive the one matching the policy
                    for (auto sIdx = 0; sIdx < notes.size(); sIdx++) {
                        if (notes.at(sIdx).state & NoteState::STOLEN) {
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
                    voices[voiceIdx]->mpe.glide(channelData[toRevive.channel].x);
                    voices[voiceIdx]->mpe.slide(channelData[toRevive.channel].y);
                    voices[voiceIdx]->mpe.press(channelData[toRevive.channel].z);
                }

            }
        }
        notes.erase(notes.begin() + idx);
    }
    else {
        auto& n = notes.at(idx);
        n.state = (NoteState)(n.state | NoteState::SUSTAINED);
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

bool inline noteIsNotStolen(const NoteData& n) {
    return (n.state & NoteState::STOLEN) == 0;
}

void VoiceBus::handleX(int channel, float position, std::vector<SynthVoice*> &voices) {
    channelData[channel].x = position;
    for (const auto &n : notes) {
        if (n.channel == channel && noteIsNotStolen(n)) {
            if (n.voiceIndex != -1) {
                voices[n.voiceIndex]->mpe.glide(position);
            }
        }
    }
}

void VoiceBus::handleY(int channel, float position, std::vector<SynthVoice*> &voices) {
    channelData[channel].y = position;
    for (const auto &n : notes) {
        if (n.channel == channel && noteIsNotStolen(n)) {
            if (n.voiceIndex != -1) {
                voices[n.voiceIndex]->mpe.slide(position);
            }
        }
    }
}

void VoiceBus::handleZ(int channel, float position, std::vector<SynthVoice*> &voices) {
    channelData[channel].z = position;
    for (const auto &n : notes) {
        if (n.channel == channel && noteIsNotStolen(n)) {
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
        if (notes[idx].voiceIndex != -1 && noteIsNotStolen(notes[idx])) {
            voices[notes[idx].voiceIndex]->mpe.press(position);
        }
    }
}

void VoiceBus::handleSustain(float position, std::vector<SynthVoice*> &voices) {
    if (sustain >= 0.5f && position < 0.5f) {
        for (auto it = notes.begin(); it != notes.end();) {
            if (it->state & NoteState::SUSTAINED) {
                if (it->voiceIndex != -1 && it->state == NoteState::SUSTAINED) {
                    voices[it->voiceIndex]->mpe.off(it->note, 0.0f);
                }
                it = notes.erase(it);
            }
            else {
                it++;
            }
        }
    }
    sustain = position;
}

int VoiceBus::getNoteDataIndex(int channel, int note) {
    int i = 0;
    int idx = -1;
    for (const auto &n : notes) {
        if (n.note == note && n.channel == channel) {
            if (idx == -1) idx = i;
            else assert(0);
        }
        i++;
    }
    return idx;
}

int VoiceBus::getNoteDataIndexForStealingVoice(int voiceIdx) {
    int i = 0;
    int idx = -1;
    for (const auto &n : notes) {
        if (n.voiceIndex == voiceIdx && noteIsNotStolen(n)) {
            if (idx == -1) idx = i;
            else assert(0);
        }
        i++;
    }
    return idx;
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

bool VoiceBus::sanitize(const std::vector<SynthVoice*> &voices) {
    int numActiveNotes = 0;
    for (int i = 0; i < notes.size(); i++) {
        const auto& n = notes.at(i);
        assert(n.state >= NoteState::ON && n.state <= NoteState::SUSTAINED_AND_STOLEN);
        numActiveNotes += noteIsNotStolen(n);
        for (int j = i + 1; j < notes.size(); j++) {
            const auto& nn = notes.at(j);
            assert( !(n.channel == nn.channel && n.note == nn.note) );
            assert( n.voiceIndex == -1 || !( noteIsNotStolen(n) && noteIsNotStolen(nn) && n.voiceIndex == nn.voiceIndex) );
        }
    }
    assert( numActiveNotes <= voices.size() );
    
    int numActiveVoices = 0;
    for (int i = 0; i < voices.size(); i++) {
        numActiveVoices += voices.at(i)->mpe.getState().gateTarget > 0;
    }
    assert( numActiveNotes == numActiveVoices );

    return true;
}

void VoiceBus::update(const std::vector<SynthVoice*> &voices) {
    assert(sanitize(voices));
}

}
