#pragma once

#include <thread>
#include <atomic>
#include "MPEVoice.hpp"
#include "module.hpp"
#include "connectiongraph.hpp"
#include "moduleregister.hpp"
#include "PhasePhckr.h"
#include "BusModules.hpp"

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

public:
    MPEVoice mpe;
    SynthVoice();
    virtual void update(float * bufferL, float * bufferR, int numSamples, float sampleRate, const GlobalData& g);
};

}

