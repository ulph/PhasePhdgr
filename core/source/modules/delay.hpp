#ifndef DELAY_HPP
#define DELAY_HPP

#include <string.h>

#include "module.hpp"
#include "sinc.hpp"
#include "inlines.hpp"
#include "rlc.hpp"

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
    float slewedTime;
public:
    Delay(const FractionalSincTable<N>& table)
        : bufferSize(0)
        , buffer(nullptr)
        , readPosition(0)
        , c_table(table)
        , slewedTime(0.f)
    {
        Module::inputs.push_back(Pad("in"));
        Module::inputs.push_back(Pad("time", 0.5f, "seconds"));
        Module::inputs.push_back(Pad("gain", 1.0f));
        Module::inputs.push_back(Pad("clear"));
        Module::inputs.push_back(Pad("compensation", 0.0f, "samples"));
        Module::inputs.push_back(Pad("timeSlewWc", 2.0f, "hz"));
        Module::outputs.push_back(Pad("out"));
    };

    Delay(const Delay<N>& other)
        : ModuleCRTP<Delay<N>>(other)
        , bufferSize(0)
        , buffer(nullptr)
        , readPosition(0)
        , c_table(other.c_table)
        , slewedTime(0.0f)
    {
    }

    virtual ~Delay() {
        delete[] buffer;
    };

    void processSample(int sample) override {
        // design a FIR from windowed sinc with fractional delay as an approx of ideal allpass

        if (clearFlag < 0.f && Module::inputs[3].values[sample] >= 0) {
            memset(buffer, 0, sizeof(float)*bufferSize);
        }
        clearFlag = Module::inputs[3].values[sample];

        float target_t = limit(Module::inputs[1].values[sample], 0, c_max_delay_t);
        float alphaTime = DesignRcLp(Module::inputs[5].values[sample], Module::fsInv);
        slewedTime = alphaTime*target_t + (1.0f - alphaTime)*slewedTime;
        float t = slewedTime;
        float g = Module::inputs[2].values[sample];
        float s = Module::inputs[4].values[sample];

        // account for filter delay
        float tapeSamples = (t*Module::fs) - (N-1) * 0.5f + s; // subtract nominal delay of filter and sample compensation
        tapeSamples = tapeSamples < 0.0f ? 0.0f : tapeSamples;
        int numSamplesTotal = (int)(ceilf(t)*Module::fs + N);
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
            buffer[(writePosition + n) % bufferSize] += coeffs[n] * g*Module::inputs[0].values[sample];
        }

        Module::outputs[0].values[sample] = buffer[readPosition];
        buffer[readPosition] = 0;
        readPosition++;
        readPosition %= bufferSize;
    };
    
    static Module* factory() { return new Delay<N>(getFractionalSincTable<N>()); }
};

class UnitDelay : public ModuleCRTP<UnitDelay> {
private:
    float last = 0.f;
public:
    UnitDelay() {
        inputs.push_back(Pad("in"));
        inputs.push_back(Pad("gain", 1.0f));
        outputs.push_back(Pad("out"));
    }
    void processSample(int sample) override {
        outputs[0].values[sample] = last;
        last = inputs[0].values[sample] * inputs[1].values[sample];
    }
    static Module* factory() { return new UnitDelay(); }
};

#endif
