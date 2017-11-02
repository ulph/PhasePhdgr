#pragma once

namespace PhasePhckr {

    class Scope {
    private:
        float scopeBuffer[512];
        const size_t scopeBufferSize;
        unsigned int scopeBufferWriteIndex;
        float scopeDrift;
    public:
        Scope();
        const float* getBuffer(int* size) const;
        void writeToBuffer(const float * leftChannelbuffer, int numSamples, float sampleRate, float hz);
    };

}