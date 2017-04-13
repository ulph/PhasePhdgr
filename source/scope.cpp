#include "PhasePhckr.h"

namespace PhasePhckr {

Scope::Scope()
    : scopeBufferWriteIndex(0)
    , scopeBufferSize(sizeof(scopeBuffer) / sizeof(float))
    , scopeDrift(0.0f)
{
    memset(scopeBuffer, 0, sizeof(scopeBuffer));
}

void Scope::writeToBuffer(float * sourceBuffer, int numSamples, float sampleRate, float hz) {
    // fill scope buffer with a (poorly) resampled version matching a couple of cycles
    if (hz > 1) {
        unsigned int numPeriods = 2;
        float samplesPerPeriod = sampleRate / hz;
        float decimation = (float)numPeriods * samplesPerPeriod / (float)scopeBufferSize;
        float i = 0;
        for (i = 0; i<numSamples; i += decimation) {
            scopeBuffer[scopeBufferWriteIndex] = sourceBuffer[(unsigned int)i];
            scopeBufferWriteIndex++;
            scopeBufferWriteIndex %= scopeBufferSize;
        }

        // compensate for rounding errors (aka, drift)
        scopeDrift += ((float)numSamples - i) / decimation;
        if (scopeDrift >= 1) {
            scopeDrift--;
            scopeBuffer[scopeBufferWriteIndex] = sourceBuffer[numSamples - numSamples>1 ? 1 : 0];
            scopeBufferWriteIndex++;
            scopeBufferWriteIndex %= scopeBufferSize;
        }
        else if (scopeDrift <= -1) {
            scopeDrift++;
            scopeBufferWriteIndex--;
            scopeBufferWriteIndex %= scopeBufferSize;
        }
    }
    else {
        scopeBufferWriteIndex = 0;
        memset(scopeBuffer, 0, sizeof(scopeBuffer));
    }
}

size_t Scope::copyBuffer(float *buffer, size_t bufferSizeIn) const
{
    size_t size = bufferSizeIn > scopeBufferSize ? scopeBufferSize : bufferSizeIn;
    for (int i = 0; i < size; i++) {
        buffer[i] = scopeBuffer[i];
    }
    return size;
}

}