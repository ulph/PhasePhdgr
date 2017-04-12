#include "camelenvelope.hpp"

int main(int argc, char *argv[])
{
    CamelEnvelope camel;
    float fs = 48000;

    FILE * outfile = stdout;
#ifdef _MSC_VER
    fprintf(stderr, "Windows can't pipe raw bytes on stdout, writing to 'data.dat' instead.");
    outfile = fopen("data.dat", "wb");
#endif

    float timescale = 40;

    camel.setInput(1, 0.5f);
    camel.setInput(2, timescale*0.025f);
    camel.setInput(3, timescale*0.05f);

    camel.setInput(4, 0.5f);

    camel.setInput(5, 0.05f);
    camel.setInput(6, timescale*0.025f);
    camel.setInput(7, timescale*0.05f);

    camel.setInput(8, 0.5f);
    camel.setInput(9, 2.f);

    camel.setInput(10, 4.0f);
    camel.setInput(11, 4.0f);

    for (uint32_t t = 0; t < 10*fs; t++) {
        float gate = t<5*fs;
        camel.setInput(0, gate);
        camel.process(fs);
        float output = camel.getOutput(0);
        fwrite(&output, sizeof(output), 1, outfile);
    }

    return 0;
}
