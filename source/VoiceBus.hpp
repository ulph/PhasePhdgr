#pragma once

#include "SynthVoice.hpp"

namespace PhasePhckr {

struct NoteData;
struct ChannelData;

class VoiceBus {
public:
    VoiceBus();
    virtual ~VoiceBus();
    void handleNoteOnOff(int channel, int note, float velocity, bool on, std::vector<SynthVoice*> &voices);
    void handleX(int channel, float position, std::vector<SynthVoice*> &voices);
    void handleY(int channel, float position, std::vector<SynthVoice*> &voices);
    void handleZ(int channel, float position, std::vector<SynthVoice*> &voices);
    void handleNoteZ(int channel, int note, float position, std::vector<SynthVoice*> &voices);
    void update();
    float findScopeVoiceHz(std::vector<SynthVoice*> &voices);
    void reset();
private:
    std::vector<NoteData*> notes;
    int getNoteDataIndex(int channel, int note);
    ChannelData *channelData;
    float scopeHz;
};

}
