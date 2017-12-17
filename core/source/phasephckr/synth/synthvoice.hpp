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
    vector<ConnectionGraph::SampleBuffer> inBuffers;
    vector<ConnectionGraph::SampleBuffer> outBuffers;
    float internalBuffer[2][SYNTH_VOICE_BUFFER_LENGTH] = { 0.0f };
    map<string, int> moduleHandles;
    ParameterHandleMap parameterHandles;
    bool buffersSilenced = false;

public:
    SynthVoice(const PatchDescriptor& voiceChain, const ComponentRegister & cp);
    virtual ~SynthVoice();
    virtual void processingStart(int numSamples, float sampleRate, const GlobalData& g);
    void threadedProcess();
    virtual void processingFinish(float * bufferL, float * bufferR, int numSamples);
    const float* getInternalBuffer(int channel) { return &internalBuffer[channel][0]; }
    void setParameter(int handle, float value);
    const ParameterHandleMap& getParameterHandles();
    MPEVoice mpe;
    bool isSilent() const;
    float getRms() const;
    void preCompile(float fs);
};

}
