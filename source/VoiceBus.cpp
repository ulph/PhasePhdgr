#include "PhasePhckr.h"
#include "SynthVoice.hpp"
#include "VoiceBus.hpp"
#include <string>

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
            if (oldestIdx == -1) {
                return; // TODO -- steal a voice
            }
            n->voiceIndex = oldestIdx;
        }
        SynthVoice *v = (*voices)[n->voiceIndex];
        v->mpe.on(note, velocity);
        v->mpe.glide(channelData[channel].x);
        v->mpe.slide(channelData[channel].y);
        v->mpe.press(channelData[channel].z);
    }
    else {
        if (idx != -1) {
            NoteData* n = notes[idx];
            if (n->voiceIndex != -1) {
                SynthVoice *v = (*voices)[n->voiceIndex];
                v->mpe.off(note, velocity);
                // wake up the youngest but paused note (if any) on the same channel -- TODO across channels
                int new_idx = findYoungestInactiveNoteDataIndex(channel);
                if (new_idx != -1) {
                    n = notes[new_idx];
                    v->mpe.on(n->note, n->velocity);
                    v->mpe.press(n->notePressure);
                    n->voiceIndex = idx;
                }
            }
            delete notes[idx];
            notes.erase(notes.begin() + idx);
        }
    }
}

void VoiceBus::handleX(int channel, float position) {
    channelData[channel].x = position;
    for (const auto &n : notes) {
        if (n->channel == channel) {
            if (n->voiceIndex != -1) {
                (*voices)[n->voiceIndex]->mpe.glide(position);
            }
        }
    }
}

void VoiceBus::handleY(int channel, float position) {
    channelData[channel].y = position;
    for (const auto &n : notes) {
        if (n->channel == channel) {
            if (n->voiceIndex != -1) {
                (*voices)[n->voiceIndex]->mpe.slide(position);
            }
        }
    }
}

void VoiceBus::handleZ(int channel, float position) {
    channelData[channel].z = position;
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
    int idx = getNoteDataIndex(channel, note);
    if (idx != -1) {
        notes[idx]->notePressure = position;
        if (notes[idx]->voiceIndex != -1) {
            (*voices)[notes[idx]->voiceIndex]->mpe.press(position);
        }
    }
}

void VoiceBus::handleExpression(float value) {
    globalDataTarget.exp = value;
}

void VoiceBus::handleBreath(float value) {
    globalDataTarget.brt = value;
}

void VoiceBus::handleModWheel(float value) {
    globalDataTarget.mod = value;
}

const GlobalData& VoiceBus::getGlobalData() {
    return globalData;
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
        if (n->channel == channel && n->voiceIndex == -1 && (int)n->age < min) {
            min = idx;
        }
        idx++;
    }
    return min;
}

float VoiceBus::findScopeVoiceHz() {
    float hz = 0;
    unsigned int max = 0;
    for (const auto &v : *voices) {
        if(v->mpe.getState().gate && v->mpe.getAge() > max){
            hz = v->mpe.getState().pitchHz;
        }
    }
    return hz;
}

void VoiceBus::update() {
    for (const auto &n : notes) {
        n->age++;
    }
    globalData.exp = globalData.exp*globalDataSlewFactor + (1 - globalDataSlewFactor)*globalDataTarget.exp;
    globalData.brt = globalData.brt*globalDataSlewFactor + (1 - globalDataSlewFactor)*globalDataTarget.brt;
    globalData.mod = globalData.mod*globalDataSlewFactor + (1 - globalDataSlewFactor)*globalDataTarget.mod;
}

VoiceBus::VoiceBus(std::vector<SynthVoice*> * parent_voices)
{
    voices = parent_voices;
}

VoiceBus::~VoiceBus() {
    for (auto &n : notes) {
        delete(n);
    }
}

}
