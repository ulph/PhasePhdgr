#pragma once

#include "synthvoice.hpp"

#include <stack>
#include <vector>

namespace PhasePhckr {

enum NoteState {
    ON = 0b00,
    SUSTAINED = 0b01,
    STOLEN = 0b10,
    SUSTAINED_AND_STOLEN = 0b11
};

struct NoteDataKey {
    int channel = -1;
    int note = -1;
    NoteDataKey() {}
    NoteDataKey(int channel, int note)
        : channel(channel)
        , note(note)
    {}
    bool operator<(const NoteDataKey& rhs) const
    {
        return std::tie(channel, note) < std::tie(rhs.channel, rhs.note);
    }
    bool isValid() const {
        return channel >= 0 && channel < 16 && note >= 0 && note < 128;
    }
};

struct NoteDataValue {
    NoteState state = NoteState::ON;
    int voiceIndex = -1;
    float notePressure = 0;
    float velocity = 0;
    NoteDataValue() {}
    NoteDataValue(float velocity)
        : velocity(velocity)
    {}
};

struct ChannelData {
    ChannelData() 
        : x(0.0f)
        , y(0.0f)
        , z(0.0f)
    {}
    float x;
    float y;
    float z;
};

class VoiceBus {
public:
    void handleNoteOnOff(int channel, int note, float velocity, bool on, std::vector<SynthVoice*> &voices);
    void handleX(int channel, float position, std::vector<SynthVoice*> &voices);
    void handleY(int channel, float position, std::vector<SynthVoice*> &voices);
    void handleZ(int channel, float position, std::vector<SynthVoice*> &voices);
    void handleNoteZ(int channel, int note, float position, std::vector<SynthVoice*> &voices);
    void handleSustain(float position, std::vector<SynthVoice*> &voices);
    void update(const std::vector<SynthVoice*> &voices);
    int findScopeVoiceIndex(std::vector<SynthVoice*> &voices);
    void setLegato(LegatoMode status) { legato = status; }
    void setStealPolicy(NoteStealPolicy newPolicy) { stealPolicy = newPolicy; }
    void setReactivationPolicy(NoteReactivationPolicy newPolicy) { reactivationPolicy = newPolicy; }
    void setActivationPolicy(NoteActivationPolicy newPolicy) { activationPolicy = newPolicy; }
    void reassignVoices(std::vector<SynthVoice*> &voices);
private:
    enum class fvr {
        NewVoice,
        StolenVoice,
        NoVoice,
        WaitingForVoice
    };
    fvr findVoice(int note, NoteDataValue* noteData, const std::vector<SynthVoice*> &voices);
    void handleNoteOn(int channel, int note, float velocity, std::vector<SynthVoice*> &voices);
    void handleNoteOff(int channel, int note, float velocity, std::vector<SynthVoice*> &voices);
    NoteStealPolicy stealPolicy = NoteStealPolicyNone;
    NoteReactivationPolicy reactivationPolicy = NoteReactivationPolicyNone;
    NoteActivationPolicy activationPolicy = NoteActivationPolicyOldest;
    LegatoMode legato = LegatoModeRetrigger;
    std::map<NoteDataKey, NoteDataValue> notes;
    ChannelData channelData[16];
    float sustain = 0.f;
    bool sanitize(const std::vector<SynthVoice*> &voices);
};

}
