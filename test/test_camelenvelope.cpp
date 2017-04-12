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

    camel.setInput(1, 0.5f);
    camel.setInput(2, 0.05f);
    camel.setInput(3, 0.1f);

    camel.setInput(4, 0.5f);

    camel.setInput(5, 0.05f);
    camel.setInput(6, 0.05f);
    camel.setInput(7, 0.5f);

    camel.setInput(8, 0.5f);
    camel.setInput(9, 2.f);

    camel.setInput(10, 4.0f);
    camel.setInput(11, 1.5f);

    for (uint32_t t = 0; t < 2*fs; t++) {
        float gate = t<fs;
        camel.setInput(0, gate);
        camel.process(fs);
        float output = camel.getOutput(0);
        fwrite(&output, sizeof(output), 1, outfile);
    }

    for (uint32_t t = 0; t < 2*fs; t++) {
        float gate = t<0.1f*fs;
        camel.setInput(0, gate);
        camel.process(fs);
        float output = camel.getOutput(0);
        fwrite(&output, sizeof(output), 1, outfile);
    }

    camel.setInput(5, 0.0f);
    camel.setInput(6, 0.0f);

    for (uint32_t t = 0; t < 2*fs; t++) {
        float gate = t<fs;
        camel.setInput(0, gate);
        camel.process(fs);
        float output = camel.getOutput(0);
        fwrite(&output, sizeof(output), 1, outfile);
    }

    for (uint32_t t = 0; t < 2*fs; t++) {
        float gate = t<0.1f*fs;
        camel.setInput(0, gate);
        camel.process(fs);
        float output = camel.getOutput(0);
        fwrite(&output, sizeof(output), 1, outfile);
    }

    return 0;
}
