#include <cstdio>
#include <cstdint>
#include "SynthVoice.hpp"

using namespace PhasePhckr;

int main()
{
    float bufferL[SYNTH_VOICE_BUFFER_LENGTH];
    float bufferR[SYNTH_VOICE_BUFFER_LENGTH];

    SynthVoice v;
    GlobalData g;
    
    v.mpe.on(48, 0.5f);
    v.mpe.press(0.5);
    
    for(int i = 0; i < 1*48000; i += SYNTH_VOICE_BUFFER_LENGTH) {
        memset(bufferL, 0, sizeof(float)*SYNTH_VOICE_BUFFER_LENGTH);
        memset(bufferR, 0, sizeof(float)*SYNTH_VOICE_BUFFER_LENGTH);
        v.processingStart(SYNTH_VOICE_BUFFER_LENGTH, 48000, g);
        v.processingFinish(bufferL, bufferR, SYNTH_VOICE_BUFFER_LENGTH);
        fwrite(bufferL, sizeof(float), 48000, stdout);
    }
}
