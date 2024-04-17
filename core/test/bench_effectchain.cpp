#include <cstdio>
#include <cstdint>

// #include "phasephdgr/synth.hpp"
#include "phasephdgr/examples.hpp"

#include "effectchain.hpp"

using namespace PhasePhdgr;

int main()
{
    const int s = 60; // ~1 minutes
    const float fs = 48000;
    float bufferL[SYNTH_VOICE_BUFFER_LENGTH];
    float bufferR[SYNTH_VOICE_BUFFER_LENGTH];

    EffectChain e(PhasePhdgr::getExampleEffectChain(), PhasePhdgr::ComponentRegister(), nullptr);
    GlobalData g;

    for (int j = 0; j < s; j++)
    {
        for (int i = 0; i < (int)fs; i += SYNTH_VOICE_BUFFER_LENGTH)
        {
            memset(bufferL, 0, sizeof(float) * SYNTH_VOICE_BUFFER_LENGTH);
            memset(bufferR, 0, sizeof(float) * SYNTH_VOICE_BUFFER_LENGTH);
            e.update(bufferL, bufferR, SYNTH_VOICE_BUFFER_LENGTH, fs, g);
        }
    }
}
