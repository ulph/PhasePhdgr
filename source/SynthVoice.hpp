#pragma once

#include <thread>
#include <atomic>
#include "MPEVoice.hpp"
#include "module.hpp"
#include "connectiongraph.hpp"
#include "moduleregister.hpp"
#include "PhasePhckr.h"
#include "BusModules.hpp"

#define SYNTH_VOICE_BUFFER_LENGTH 1024

namespace PhasePhckr {

class SynthVoice {
private:
    ConnectionGraph connectionGraph;
    int inBus;
    int outBus;
    float rmsSlew;
    float rms;
    std::thread t;
    std::atomic<int> samplesToProcess;
    void threadedProcess();
    GlobalData globalData;
    float sampleRate;
    float internalBuffer[2][SYNTH_VOICE_BUFFER_LENGTH];
    bool doTerminate;

public:
    MPEVoice mpe;
    SynthVoice();
    ~SynthVoice();
    virtual void processingStart(int numSamples, float sampleRate, const GlobalData& g);
    virtual void processingFinish(float * bufferL, float * bufferR, int numSamples);
};

}

