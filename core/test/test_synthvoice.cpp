#include <cstdio>
#include <cstdint>

#include "phasephckr/synth.hpp"
#include "phasephckr/examples.hpp"

#include "synthvoice.hpp"

using namespace PhasePhckr;

int main()
{
    const float fs = 48000;
    float bufferL[SYNTH_VOICE_BUFFER_LENGTH];
    float bufferR[SYNTH_VOICE_BUFFER_LENGTH];

    SynthVoice v(PhasePhckr::getExampleVoiceChain(), PhasePhckr::ComponentRegister());
    GlobalData g;
    
    v.mpe.on(48, 0.5f);
    v.mpe.press(0.5);
    
    for (int j = 0; j < 30; j++) {
        for (int i = 0; i < 1 * (int)fs; i += SYNTH_VOICE_BUFFER_LENGTH) {
            memset(bufferL, 0, sizeof(float)*SYNTH_VOICE_BUFFER_LENGTH);
            memset(bufferR, 0, sizeof(float)*SYNTH_VOICE_BUFFER_LENGTH);
            v.processingStart(SYNTH_VOICE_BUFFER_LENGTH, fs, g);
            v.threadedProcess();
            v.processingFinish(bufferL, bufferR, SYNTH_VOICE_BUFFER_LENGTH);
        }
    }
}
