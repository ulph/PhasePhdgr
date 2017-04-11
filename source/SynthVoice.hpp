#pragma once

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

public:
    MPEVoice mpe;
    SynthVoice();
    virtual void update(float * bufferL, float * bufferR, int numSamples, float sampleRate, const GlobalData& g);
};

}

