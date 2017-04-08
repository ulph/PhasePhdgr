#pragma once

#include <math.h>
#include <vector>
#include <random>
#include <string.h>

namespace PhasePhckr {

    class SynthVoiceI;
    class EffectI;

    class VoiceBus {
        private:
        std::vector<SynthVoiceI*> *voices;

        public:
        VoiceBus() : voices(nullptr){};
        VoiceBus(std::vector<SynthVoiceI*> * parent_voices) {
            voices = parent_voices;
        }

        void handleNoteOnOff(int channel, int note, float velocity, bool on);
        void handleX(int channel, float position);
        void handleY(int channel, float position);
        void handleZ(int channel, float position);
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
