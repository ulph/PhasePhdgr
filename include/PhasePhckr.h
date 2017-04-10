#pragma once

#include <math.h>
#include <vector>
#include <utility>
#include <random>
#include <string.h>

namespace PhasePhckr {

    class SynthVoiceI;
    class EffectI;
    struct NoteData;

    struct ChannelData {
      ChannelData()
        :x(0), y(0), z(0) {}

        float x;
        float y;
        float z;
    };

    class VoiceBus {
    public:
        VoiceBus() : voices(nullptr){};
        VoiceBus(std::vector<SynthVoiceI*> * parent_voices);
        virtual ~VoiceBus();
        void handleNoteOnOff(int channel, int note, float velocity, bool on);
        void handleX(int channel, float position);
        void handleY(int channel, float position);
        void handleZ(int channel, float position);
        void handleNoteZ(int channel, int note, float position);
        void update();
    private:
        std::vector<SynthVoiceI*> *voices;
        std::vector<NoteData*> notes;
        int getNoteDataIndex(int channel, int note);
        int findYoungestInactiveNoteDataIndex(int channel);
        ChannelData channelData[16];
    };

    class AutomationBus {
        // handles the automation (global VST) parameters
    };

    class Synth {
    public:
        Synth();
        VoiceBus voiceBus;
        AutomationBus automationBus;
        virtual void update(float * buffer, int numSamples, float sampleRate);
    private:
        std::vector<SynthVoiceI*> voices; // per note sound generation
        std::vector<EffectI*> effects; // effects applied to mix of voices (in series)
        // settings like voice stacking, per voice detuning and etc etc goes here
    };

}
