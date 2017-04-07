#ifndef EXSYNTHVOICE_H_INCLUDED
#define EXSYNTHVOICE_H_INCLUDED

#include "PhasePhckr.h"
#include "Components/CamelEnvelope.h"

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

}

#endif  // EXSYNTHVOICE_H_INCLUDED
