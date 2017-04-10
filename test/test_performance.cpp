#include <cstdio>
#include <cstdint>
#include "SynthVoice.hpp"

using namespace PhasePhckr;

int main()
{
    float buffer[48000];
    
    ConnectionGraphVoice v;
    GlobalData g;
    
    v.mpe.on(48, 0.5f);
    
    for(int i = 0; i < 10; i++) {
        v.update(buffer, 48000, 48000, g);
    }
}
