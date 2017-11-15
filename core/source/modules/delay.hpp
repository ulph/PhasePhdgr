#ifndef DELAY_HPP
#define DELAY_HPP

#include "module.hpp"
#include "sinc.hpp"

const int c_fractions = 1000;
const float c_max_delay_t = 5.f;

namespace DelayFactory {
    template <int N>
    const FractionalSincTable<N, c_fractions> & getFractionalSincTable();

    Module* (*makeFactory(int numFractions)) (void);
}

template <int N>
class Delay : public ModuleCRTP<Delay<N>>
{
private:
    float *buffer;
    int bufferSize;
    int readPosition;
    const FractionalSincTable<N, c_fractions>& c_table;
public:
    Delay(const FractionalSincTable<N, c_fractions>& table)
        : buffer(nullptr)
        , bufferSize(0)
        , readPosition(0)
        , c_table(table)
    {
        inputs.push_back(Pad("in"));
        inputs.push_back(Pad("time", 0.5f));
        inputs.push_back(Pad("gain", 1.0f));
        outputs.push_back(Pad("out"));
    };

    virtual ~Delay() {
        delete[] buffer;
    };

    void process(uint32_t fs) {
        // design a FIR from windowed sinc with fractional delay as an approx of ideal allpass

        float t = inputs[1].value;
        float g = inputs[2].value;

        t = (t < 0.f) ? 0.f : (t > c_max_delay_t ? c_max_delay_t : t);

        // account for filter delay
        float tapeSamples = (t*fs) + N;
        tapeSamples = tapeSamples < N ? N : tapeSamples;

        if (tapeSamples > bufferSize) {
            // lazily grow the buffer ... TODO, proper memory chunk allocations
            auto newBufferSize = 2 * (int)tapeSamples;
            auto newBuffer = new float[newBufferSize]();
            memmove(newBuffer, buffer, sizeof(float)*bufferSize);
            delete[] buffer;
            buffer = newBuffer;
            bufferSize = newBufferSize;
        }

        tapeSamples = ((tapeSamples + N) > bufferSize) ? (bufferSize - N) : tapeSamples;
        const int writePosition = (readPosition + (int)tapeSamples);
        const float frac = tapeSamples - (int)(tapeSamples);

        // ... (interpolated) impulse response from a precalcuated buffer
        float coeffs[N] = { 0 };
        if (N != c_table.getCoefficients(frac, &coeffs[0], N)) {
            assert(0);
        }

        // apply it on to write buffer (running convolution)
        for (int n = 0; n < N; n++) {
            buffer[(writePosition + n) % bufferSize] += coeffs[n] * g*inputs[0].value;
        }

        outputs[0].value = buffer[readPosition];
        buffer[readPosition] = 0;
        readPosition++;
        readPosition %= bufferSize;
    };
    
    static Module* factory() { return new Delay<N>(DelayFactory::getFractionalSincTable<N>()); }
};

#endif
