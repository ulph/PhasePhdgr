#pragma once

#include <thread>
#include <atomic>

#include "phasephckr/components.hpp"
#include "phasephckr/design.hpp"

#include "parameters.hpp"
#include "module.hpp"
#include "connectiongraph.hpp"
#include "moduleregister.hpp"
#include "busmodules.hpp"

#define SYNTH_VOICE_BUFFER_LENGTH 1024

namespace PhasePhckr {

class SynthVoiceThreading {
    // fragmentary remain of the threading
public:
    GlobalData globalData;
    int samplesToProcess;
    float sampleRate;
};

class SynthVoice {
private:
    ConnectionGraph connectionGraph;
    int inBus;
    int outBus;
    float rmsSlew;
    float rms;
    SynthVoiceThreading threadStuff;
    void threadedProcess();
    float internalBuffer[2][SYNTH_VOICE_BUFFER_LENGTH];
    map<string, int> moduleHandles;
    ParameterHandleMap parameterHandles;
public:
    MPEVoice mpe;
    SynthVoice(const PatchDescriptor& voiceChain, const ComponentRegister & cp);
    virtual ~SynthVoice();
    virtual void processingStart(int numSamples, float sampleRate, const GlobalData& g);
    virtual void processingFinish(float * bufferL, float * bufferR, int numSamples);
    const float* getInternalBuffer(int channel) { return &internalBuffer[channel][0]; }
    void setParameter(int handle, float value);
    const ParameterHandleMap& getParameterHandles();
    bool isSilent();
};

}
