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
    
    for(int i = 0; i < 10*48000; i += SYNTH_VOICE_BUFFER_LENGTH) {
        v.processingStart(SYNTH_VOICE_BUFFER_LENGTH, 48000, g);
        v.processingFinish(bufferL, bufferR, SYNTH_VOICE_BUFFER_LENGTH);
        //fwrite(bufferL, sizeof(float), 48000, stdout);
    }
}
