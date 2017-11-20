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
    this->threadedProcess();
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
    int numSamples = threadStuff.samplesToProcess;
    if(threadStuff.samplesToProcess > 0) {
        for (int j = 0; j < numSamples; ++j) {
            mpe.update();
            threadStuff.globalData.update();
            const MPEVoiceState &v = mpe.getState();
            const GlobalDataState &g = threadStuff.globalData.getState();
            const GlobalTimeDataState &t = threadStuff.globalData.getTimeState();

            internalBuffer[0][j] = 0.0f;
            internalBuffer[1][j] = 0.0f;

            if (v.gate) {
                rms = 1;
            } else if (v.gate == 0 && isSilent()) {
                continue;
            }

            int i=0;
            connectionGraph.setInput(inBus, i++, v.gate);
            connectionGraph.setInput(inBus, i++, v.strikeZ);
            connectionGraph.setInput(inBus, i++, v.liftZ);
            connectionGraph.setInput(inBus, i++, v.pitchHz);
            connectionGraph.setInput(inBus, i++, v.glideX);
            connectionGraph.setInput(inBus, i++, v.slideY);
            connectionGraph.setInput(inBus, i++, v.pressZ);

            connectionGraph.setInput(inBus, i++, g.mod);
            connectionGraph.setInput(inBus, i++, g.exp);
            connectionGraph.setInput(inBus, i++, g.brt);

            connectionGraph.setInput(inBus, i++, (float)t.nominator);
            connectionGraph.setInput(inBus, i++, (float)t.denominator);
            connectionGraph.setInput(inBus, i++, t.barLength);
            connectionGraph.setInput(inBus, i++, t.bpm);
            connectionGraph.setInput(inBus, i++, t.barPosition);
            connectionGraph.setInput(inBus, i++, t.position);
            connectionGraph.setInput(inBus, i++, t.time);

            connectionGraph.setInput(inBus, i++, v.noteIndex);
            connectionGraph.setInput(inBus, i++, v.voiceIndex);
            connectionGraph.setInput(inBus, i++, v.polyphony);

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
