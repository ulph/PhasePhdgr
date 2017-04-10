#include <cstdio>
#include <cstdint>

#include "connectiongraph.hpp"
#include "module.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    uint32_t fs = 48000;
    ConnectionGraph s(fs);
    int phase = s.addModule("PHASE");
    int mul = s.addModule("MUL");
    s.getModule(phase)->setInput(0, 1.0);

    int dut = phase;

    FILE * outfile = fopen("data.dat", "wb"); // stupid windows, cout doesn't work

    if (argc > 1) {
        dut = s.addModule(argv[1]);
        if (dut == -1) return -1;
        s.connect(phase, dut);
        for (int i = 2; i < argc; i++) {
            int new_dut = s.addModule(argv[i]);
            if (new_dut == -1) return -1;
            s.connect(dut, new_dut);
            dut = new_dut;
        }
    }

    for (uint32_t t = 0; t < 10*fs; t++) {
        s.process(dut);
        float output = s.getOutput(dut, 0);
        fwrite(&output, sizeof(output), 1, outfile);
    }

    return 0;
}
