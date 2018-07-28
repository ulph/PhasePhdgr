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

#if SUPPORT_PLUGIN_LOADING
#include "pluginsregister.hpp"
#else
namespace PhasePhckr {
    class PluginsRegister;
};
#endif

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
    float internalBuffer[2][SYNTH_VOICE_BUFFER_LENGTH] = { { 0.0f }, { 0.0f } };
    map<string, int> moduleHandles;
    ParameterHandleMap parameterHandles;
    bool buffersSilenced = false;

public:
    SynthVoice(const PatchDescriptor& voiceChain, const ComponentRegister & cp, const PluginsRegister * sdkReg);
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
    void reset();
};

}
