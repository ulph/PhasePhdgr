#include "synthvoice.hpp"
#include "phasephckr.h"
#include "design.hpp"

namespace PhasePhckr
{

SynthVoice::SynthVoice(const PatchDescriptor& voiceChain, const ComponentRegister & cp)
    : connectionGraph()
    , rmsSlew(0.99f)
    , rms(0.0f)
    , samplesToProcess(0)
    , doTerminate(false)
{
    ModuleRegister::registerAllModules(connectionGraph);
    PatchDescriptor patchDescriptor = voiceChain;

    std::map<std::string, int> moduleHandles;
    designPatch(
        connectionGraph,
        patchDescriptor,
        c_voiceChainInBus,
        c_voiceChainOutBus,
        moduleHandles,
        cp
    );

    inBus = moduleHandles["inBus"];
    outBus = moduleHandles["outBus"];

#if MULTITHREADED
    t = std::thread(&SynthVoice::threadedProcess, this);
#endif
}

SynthVoice::~SynthVoice()
{
    doTerminate = true;
#if MULTITHREADED
    t.join();
#endif
}

void SynthVoice::processingStart(int numSamples, float sampleRate, const GlobalData& g)
{
#if MULTITHREADED
    // Make sure thread is not processing already ... (should not happen)
    while(samplesToProcess > 0) std::this_thread::yield();
#endif
    // Queue work for thread
    globalData = g;
    this->sampleRate = sampleRate;
    samplesToProcess = numSamples;
#if MULTITHREADED
#else
    this->threadedProcess();
#endif
}

void SynthVoice::processingFinish(float * bufferL, float * bufferR, int numSamples)
{
#if MULTITHREADED
    // Wait for thread to complete...
    while(samplesToProcess > 0) std::this_thread::yield();
#endif
    // Collect data
    for(int i = 0; i < numSamples; i++) {
        bufferL[i] += internalBuffer[0][i];
        bufferR[i] += internalBuffer[1][i];
    }
}


void SynthVoice::threadedProcess()
{
#if MULTITHREADED
    while(!doTerminate) {
#endif
        int numSamples = samplesToProcess;
        if(samplesToProcess > 0) {
            for (int i = 0; i < numSamples; ++i) {
                mpe.update();
                globalData.update();
                const MPEVoiceState &v = mpe.getState();
                const GlobalDataState &g = globalData.getState();

                internalBuffer[0][i] = 0.0f;
                internalBuffer[1][i] = 0.0f;

                if (v.gate) {
                    rms = 1;
                } else if (v.gate == 0 && rms < 0.0000001) {
                    continue;
                }

                connectionGraph.setInput(inBus, 0, v.gate);
                connectionGraph.setInput(inBus, 1, v.strikeZ);
                connectionGraph.setInput(inBus, 2, v.liftZ);
                connectionGraph.setInput(inBus, 3, v.pitchHz);
                connectionGraph.setInput(inBus, 4, v.glideX);
                connectionGraph.setInput(inBus, 5, v.slideY);
                connectionGraph.setInput(inBus, 6, v.pressZ);
                connectionGraph.setInput(inBus, 7, g.mod);
                connectionGraph.setInput(inBus, 8, g.exp);
                connectionGraph.setInput(inBus, 9, g.brt);

                connectionGraph.process(outBus, sampleRate);
                float sampleL = connectionGraph.getOutput(outBus, 0);
                float sampleR = connectionGraph.getOutput(outBus, 1);
                internalBuffer[0][i] = sampleL;
                internalBuffer[1][i] = sampleR;
                rms = rms*rmsSlew + (1 - rmsSlew)*((sampleL+sampleR)*(sampleL+sampleR)); // without the root
            }

            samplesToProcess -= numSamples;
        } else {
#if MULTITHREADED
            std::this_thread::yield();
#endif
        }
#if MULTITHREADED
    }
#endif
}

}
