#ifndef DELAY_HPP
#define DELAY_HPP

#include "module.hpp"
#include "sinc.hpp"
#include "inlines.hpp"

const float c_max_delay_t = 5.f;
const float c_min_delay_t = 0.0005f; // 0.5f * 32.f / 32000.f; // half of max sinc window for 32kHz

namespace DelayFactory {
    Module* (*makeFactory(int numFractions)) (void);
}

template <int N>
class Delay : public ModuleCRTP<Delay<N>>
{
private:
    float *buffer;
    int bufferSize;
    int readPosition;
    float coeffs[N];
    const FractionalSincTable<N>& c_table;
    float clearFlag;
public:
    Delay(const FractionalSincTable<N>& table)
        : bufferSize(0)
        , buffer(nullptr)
        , readPosition(0)
        , c_table(table)
    {
        Module::inputs.push_back(Pad("in"));
        Module::inputs.push_back(Pad("time", 0.5f));
        Module::inputs.push_back(Pad("gain", 1.0f));
        Module::inputs.push_back(Pad("clear"));
        Module::outputs.push_back(Pad("out"));
    };

    virtual ~Delay() {
        delete[] buffer;
    };

    void process() {
        // design a FIR from windowed sinc with fractional delay as an approx of ideal allpass

        if (clearFlag < 0.f && Module::inputs[3].value >= 0) {
            memset(buffer, 0, sizeof(float)*bufferSize);
        }
        clearFlag = Module::inputs[3].value;

        float t = limit(Module::inputs[1].value, 0, c_max_delay_t);
        float g = Module::inputs[2].value;

        // account for filter delay
        float tapeSamples = (t*fs) - N*0.5f; // subtract nominal delay of filter
        tapeSamples = tapeSamples < 0.0f ? 0.0f : tapeSamples;
        int numSamplesTotal = (int)(ceilf(t)*fs + N);
        if (numSamplesTotal >= bufferSize) {
            // lazily grow the buffer ...
            auto newBufferSize = 2 * numSamplesTotal;
            auto newBuffer = new float[newBufferSize]();
            memset(newBuffer, 0, sizeof(float)*newBufferSize);
            memmove(newBuffer, buffer, sizeof(float)*bufferSize);
            delete[] buffer;
            buffer = newBuffer;
            bufferSize = newBufferSize;
        }

        const int writePosition = (readPosition + (int)tapeSamples);
        const float frac = tapeSamples - (int)(tapeSamples);

        float *coeffs = nullptr;
        auto ret = c_table.getCoefficientTablePointer(frac, &coeffs, N);
        assert(ret == N);
        assert(coeffs != nullptr);

        // apply it on to write buffer (running convolution)
        for (int n = 0; n < N; n++) {
            buffer[(writePosition + n) % bufferSize] += coeffs[n] * g*Module::inputs[0].value;
        }

        Module::outputs[0].value = buffer[readPosition];
        buffer[readPosition] = 0;
        readPosition++;
        readPosition %= bufferSize;
    };
    
    static Module* factory() { return new Delay<N>(getFractionalSincTable<N>()); }
};

#endif
