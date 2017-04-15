#include <cstdio>
#include <cstdint>
#include "connectiongraph.hpp"
#include "module.hpp"
#include "moduleregister.hpp"

void connect_design_to_filter(ConnectionGraph& cg, int design, int filter)
{
    cg.connect(design, "a1", filter, "a1");
    cg.connect(design, "a2", filter, "a2");
    cg.connect(design, "b0", filter, "b0");
    cg.connect(design, "b1", filter, "b1");
    cg.connect(design, "b2", filter, "b2");
}

int main()
{
    uint32_t fs = 48000;
    ConnectionGraph s;
    ModuleRegister::registerAllModules(s);

    int out = -1;

    int noise = s.addModule(("NOISE"));

    int lpf = s.addModule("LPF");
    int biquad_lp = s.addModule("BIQUAD");
    connect_design_to_filter(s, lpf, biquad_lp);

    s.connect(noise, "out", biquad_lp, "input");

    s.setInput(lpf, "f0", 20.f);

    int peak = s.addModule("PEAKEQ");

    int biquad_peak = s.addModule("BIQUAD");
    connect_design_to_filter(s, peak, biquad_peak);
    s.connect(noise, "out", biquad_peak, "input");

    s.setInput(peak, "f0", 5000.);
    s.connect(biquad_lp, "output", peak, "f0");

    s.setInput(peak, "Q", .5);
    s.setInput(peak, "A", -6.0);

    out = biquad_peak;

    std::cerr << s.graphviz() << std::endl;

    for(uint32_t t = 0; t < 5*fs; t++) {
        s.process(out, fs);
        float output = s.getOutput(out, 0);
        fwrite(&output, sizeof(output), 1, stdout); 
    }
}
