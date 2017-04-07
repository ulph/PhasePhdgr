#ifndef EXSYNTHVOICE_H_INCLUDED
#define EXSYNTHVOICE_H_INCLUDED

#include "../PhasePhckr.h"
#include "../Components/CamelEnvelope.h"

namespace PhasePhckr {

    // example stupid stuff to have _something_ working
    class ExSynthVoice : public SynthVoiceI {
      public:
        virtual void reset();
        virtual void update(float * buffer, int numSamples, float sampleRate);

      private:
        Components::CamelEnvelope env;
        Components::CamelEnvelope noiseEnv;
        long double angle;
    };

    class Synth {
    public:
        Synth() {
            for(int i=0; i<16; ++i){
                SynthVoiceI* v = new ExSynthVoice();
                v->reset();
                voices.push_back(v);
            }
            voiceBus = VoiceBus(&voices);
        }
        VoiceBus voiceBus;
        AutomationBus automationBus;
        virtual void update(float * buffer, int numSamples, float sampleRate) {
            for (auto & v : voices) v->update(buffer, numSamples, sampleRate);
            for (auto & e : effects) e->update(buffer, numSamples, sampleRate);
        }
    private:
        std::vector<SynthVoiceI*> voices; // per note sound generation
        std::vector<SynthVoiceI*> effects; // effects applied to mix of voices (in series)
        // settings like voice stacking, per voice detuning and etc etc goes here
    };

}

#endif  // EXSYNTHVOICE_H_INCLUDED
