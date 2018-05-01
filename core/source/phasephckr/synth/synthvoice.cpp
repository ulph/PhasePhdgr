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
        patchDescriptor.componentBundle,
        c_voiceChainInBus,
        c_voiceChainOutBus,
        moduleHandles,
        SynthGraphType::VOICE,
        parameterHandles,
        cp
    );

    inBus = moduleHandles["inBus"];
    outBus = moduleHandles["outBus"];
}

void SynthVoice::preCompile(float fs) {
    connectionGraph.compileProgram(outBus);
}

SynthVoice::~SynthVoice()
{
}

void SynthVoice::processingStart(int numSamples, float sampleRate, const GlobalData& g)
{
    for(const auto& p: parameterHandles){
        connectionGraph.setInput(p.first, 0, p.second.v.val);
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

bool SynthVoice::isSilent() const {
    return rms < 0.000000001;
}

float SynthVoice::getRms() const {
    return rms;
}

void SynthVoice::threadedProcess()
{
    int numSamples = threadStuff.samplesToProcess;
    assert(numSamples % ConnectionGraph::k_blockSize == 0);

    const MPEVoiceState &v = mpe.getState();
    if (v.gateTarget) {
        rms = 1;
        buffersSilenced = false;
    }
    else if (v.gateTarget == 0 && isSilent()) {
        if (!buffersSilenced) {
            for (int i = 0; i < SYNTH_VOICE_BUFFER_LENGTH; i++) {
                internalBuffer[0][i] = 0.f;
                internalBuffer[1][i] = 0.f;
            }
        }
        buffersSilenced = true;
        return;
    }

    const GlobalDataState &g = threadStuff.globalData.getState();
    const GlobalTimeDataState &t = threadStuff.globalData.getTimeState();

    connectionGraph.setInput(inBus, 1, v.strikeZ);
    connectionGraph.setInput(inBus, 2, v.liftZ);

    connectionGraph.setInput(inBus, 10, (float)t.nominator);
    connectionGraph.setInput(inBus, 11, (float)t.denominator);
    connectionGraph.setInput(inBus, 12, t.barLength);
    connectionGraph.setInput(inBus, 13, t.bpm);
    connectionGraph.setInput(inBus, 14, t.barPosition);
    connectionGraph.setInput(inBus, 15, t.position);
    connectionGraph.setInput(inBus, 16, t.time);

    connectionGraph.setInput(inBus, 18, v.noteIndex2);
    connectionGraph.setInput(inBus, 19, v.voiceIndex);
    connectionGraph.setInput(inBus, 20, v.polyphony);

    int j = 0;
    for (j = 0; (j + ConnectionGraph::k_blockSize) <= numSamples; j += ConnectionGraph::k_blockSize) {
        bool gateChange = mpe.gateChanged();

        mpe.update();
        threadStuff.globalData.update();

        if(gateChange) connectionGraph.setInputBlock(inBus, 0, v.gate);

        connectionGraph.setInputBlock(inBus, 3, v.pitchHz);
        connectionGraph.setInputBlock(inBus, 4, v.glideX);
        connectionGraph.setInputBlock(inBus, 5, v.slideY);
        connectionGraph.setInputBlock(inBus, 6, v.pressZ);

        connectionGraph.setInputBlock(inBus, 7, g.mod);
        connectionGraph.setInputBlock(inBus, 8, g.exp);
        connectionGraph.setInputBlock(inBus, 9, g.brt);

        connectionGraph.setInputBlock(inBus, 17, v.noteIndex);

        connectionGraph.processBlock(outBus, threadStuff.sampleRate);

        connectionGraph.getOutputBlock(outBus, 0, &internalBuffer[0][j]);
        connectionGraph.getOutputBlock(outBus, 1, &internalBuffer[1][j]);

        for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
            float v = internalBuffer[0][j+i]*internalBuffer[0][j+i] + internalBuffer[1][j+i]*internalBuffer[1][j+i];
            rms = rms*rmsSlew + (1 - rmsSlew)*v;
        }

    }

    assert(j == numSamples);
}

void SynthVoice::setParameter(int handle, float value){
    auto it = parameterHandles.find(handle);
    if(it == parameterHandles.end()){
        assert(0);
        return;
    }
    it->second.v.val = value;
}

const ParameterHandleMap& SynthVoice::getParameterHandles(){
    return parameterHandles;
}

}
