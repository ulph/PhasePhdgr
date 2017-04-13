#include <cstdio>
#include <cstdint>
#include "connectiongraph.hpp"
#include "module.hpp"
#include "moduleregister.hpp"

int main()
{
    uint32_t fs = 48000;
    ConnectionGraph s;
    ModuleRegister::registerAllModules(s);

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
    s.connect(q8, "quant", speaker, "in");

    int noise = s.addModule(("NOISE"));
    int lpf = s.addModule("LPF");
    int biquad = s.addModule("BIQUAD");

    s.connect(noise, "out", biquad, "input");
    s.connect(lpf, "a1", biquad, "a1");
    s.connect(lpf, "a2", biquad, "a2");
    s.connect(lpf, "b0", biquad, "b0");
    s.connect(lpf, "b1", biquad, "b1");
    s.connect(lpf, "b2", biquad, "b2");
    speaker = biquad;
    s.setInput(lpf, "f0", 4000.f);

    for(uint32_t t = 0; t < 5*fs; t++) {
        s.process(speaker, fs);
        float output = s.getOutput(speaker, 0);
        fwrite(&output, sizeof(output), 1, stdout); 
    }
}
