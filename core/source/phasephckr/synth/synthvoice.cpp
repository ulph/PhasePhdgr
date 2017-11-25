#include "synthvoice.hpp"

namespace PhasePhckr
{


SynthVoice::SynthVoice(const PatchDescriptor& voiceChain, const ComponentRegister & cp)
    : connectionGraph()
    , rmsSlew(0.99f)
    , rms(0.0f)
{
    ModuleRegister::registerAllModules(connectionGraph);
    PatchDescriptor patchDescriptor = voiceChain;

    moduleHandles.clear();
    parameterHandles.clear();

    designPatch(
        connectionGraph,
        patchDescriptor,
        c_voiceChainInBus,
        c_voiceChainOutBus,
        moduleHandles,
        VOICE,
        parameterHandles,
        cp
    );

    inBus = moduleHandles["inBus"];
    outBus = moduleHandles["outBus"];
}

SynthVoice::~SynthVoice()
{
}

void SynthVoice::processingStart(int numSamples, float sampleRate, const GlobalData& g)
{
    for(const auto& p: parameterHandles){
        connectionGraph.setInput(p.first, 0, p.second.val);
    }
    // Queue work for thread
    threadStuff.globalData = g;
    threadStuff.sampleRate = sampleRate;
    threadStuff.samplesToProcess = numSamples;
}

void SynthVoice::processingFinish(float * bufferL, float * bufferR, int numSamples)
{
    // Collect data
    for(int i = 0; i < numSamples; i++) {
        bufferL[i] += internalBuffer[0][i];
        bufferR[i] += internalBuffer[1][i];
    }
}

bool SynthVoice::isSilent(){
    return rms < 0.0000001;
}

void SynthVoice::threadedProcess()
{
    const MPEVoiceState &v = mpe.getState();
    if (v.gate) {
        rms = 1;
    }
    else if (v.gate == 0 && isSilent()) {
        threadStuff.samplesToProcess = 0;
        return;
    }

    int numSamples = threadStuff.samplesToProcess;

    const GlobalDataState &g = threadStuff.globalData.getState();
    const GlobalTimeDataState &t = threadStuff.globalData.getTimeState();

    connectionGraph.setInput(inBus, 0, v.gate);
    connectionGraph.setInput(inBus, 1, v.strikeZ);
    connectionGraph.setInput(inBus, 2, v.liftZ);

    connectionGraph.setInput(inBus, 10, (float)t.nominator);
    connectionGraph.setInput(inBus, 11, (float)t.denominator);
    connectionGraph.setInput(inBus, 12, t.barLength);
    connectionGraph.setInput(inBus, 13, t.bpm);
    connectionGraph.setInput(inBus, 14, t.barPosition);
    connectionGraph.setInput(inBus, 15, t.position);
    connectionGraph.setInput(inBus, 16, t.time);

    connectionGraph.setInput(inBus, 17, v.noteIndex);
    connectionGraph.setInput(inBus, 18, v.voiceIndex);
    connectionGraph.setInput(inBus, 19, v.polyphony);

    if(threadStuff.samplesToProcess > 0) {
        for (int j = 0; j < numSamples; ++j) {
            mpe.update();
            threadStuff.globalData.update();

            internalBuffer[0][j] = 0.0f;
            internalBuffer[1][j] = 0.0f;

            connectionGraph.setInput(inBus, 3, v.pitchHz);
            connectionGraph.setInput(inBus, 4, v.glideX);
            connectionGraph.setInput(inBus, 5, v.slideY);
            connectionGraph.setInput(inBus, 6, v.pressZ);

            connectionGraph.setInput(inBus, 7, g.mod);
            connectionGraph.setInput(inBus, 8, g.exp);
            connectionGraph.setInput(inBus, 9, g.brt);

            connectionGraph.process(outBus, threadStuff.sampleRate);
            float sampleL = connectionGraph.getOutput(outBus, 0);
            float sampleR = connectionGraph.getOutput(outBus, 1);
            internalBuffer[0][j] = sampleL;
            internalBuffer[1][j] = sampleR;
            rms = rms*rmsSlew + (1 - rmsSlew)*((sampleL+sampleR)*(sampleL+sampleR)); // without the root
        }

        threadStuff.samplesToProcess -= numSamples;
    }
}

void SynthVoice::setParameter(int handle, float value){
    auto it = parameterHandles.find(handle);
    if(it == parameterHandles.end()){
        assert(0);
        return;
    }
    it->second.val = value;
}

const ParameterHandleMap& SynthVoice::getParameterHandles(){
    return parameterHandles;
}

}
