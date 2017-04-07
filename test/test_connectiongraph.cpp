#include <cstdio>
#include <cstdint>
#include "connectiongraph.hpp"

int main()
{
    uint32_t fs = 48000;
    ConnectionGraph s(fs);
    int osc1 = s.addModule("PHASE");
    s.setInput(osc1, 0, 440.0f);
    
    int osc2 = s.addModule("PHASE");
    s.setInput(osc2, 0, 0.5f);
    
    int square = s.addModule("SQUARE");
    s.connect(osc1, square, 0);
    s.connect(osc2, square, 1);
    
    int gain = s.addModule("MUL");
    s.setInput(gain, 0, 0.25f);
    s.connect(square, gain, 1);
    
    int q8 = s.addModule("QUANT8");
    s.connect(gain, q8);

    int speaker = s.addModule("CLAMP");
    s.connect(q8, speaker, 0);

    // int noise = s.addModule(("NOISE"));
    // s.connect(noise, speaker);
    
    for(uint32_t t = 0; t < 5*fs; t++) {
        s.process(speaker, t);
        float output = s.getOutput(speaker, 0);
        fwrite(&output, sizeof(output), 1, stdout); 
    }
}
