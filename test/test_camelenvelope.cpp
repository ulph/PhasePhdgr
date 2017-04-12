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

    camel.setInput(1, 0.5);
    camel.setInput(2, 1);
    camel.setInput(3, 1);

    camel.setInput(4, 0.1);

    camel.setInput(5, .1);
    camel.setInput(6, 1);
    camel.setInput(7, 1);

    camel.setInput(8, .5);
    camel.setInput(9, 10);

    camel.setInput(10, 2);
    camel.setInput(11, .5);

    for (uint32_t t = 0; t < 10*fs; t++) {
        float gate = t<5*fs;
        camel.setInput(0, gate);
        camel.process(fs);
        float output = camel.getOutput(0);
        fwrite(&output, sizeof(output), 1, outfile);
    }

    return 0;
}
