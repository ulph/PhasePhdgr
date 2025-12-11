#include <cstdio>
#include <cstdint>
#include "connectiongraph.hpp"
#include "moduleregister.hpp"

int main(int argc, char *argv[])
{
    uint32_t fs = 48000;
    float hz = 440.f;
    ConnectionGraph s;
    ModuleRegister::registerAllModules(s);
    int phase = s.addModule("PHASE");
    s.setInput(phase, 0, hz);

    int dut = phase;

    FILE * outfile = stdout;
#ifdef _MSC_VER
    fprintf(stderr, "Windows can't pipe raw bytes on stdout, writing to 'data.dat' instead.");
    auto err = fopen_s(&outfile, "data.dat", "wb");
    assert(outfile);
    assert(err == 0);
#endif

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

    for (uint32_t t = 0; t < 5.0f*(float(fs)/hz); t++) {
        // FIXME: Use block processing.
        //s.processSample(dut, (float)fs);
        //float output = s.getOutput(dut, 0);
        //fwrite(&output, sizeof(output), 1, outfile);
    }

    return 0;
}
