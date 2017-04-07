#pragma once

#include <math.h>
#include <vector>
#include <random>
#include <string.h>

namespace PhasePhckr {

    class SynthVoiceI;

    class VoiceBus {
        private:
        std::vector<SynthVoiceI*> *voices;

        // TODO, make it work for normal polyphony as well
        // and use internal round-robin instead of relying of channel ...

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
        std::vector<SynthVoiceI*> effects; // effects applied to mix of voices (in series)
        // settings like voice stacking, per voice detuning and etc etc goes here
    };

}
