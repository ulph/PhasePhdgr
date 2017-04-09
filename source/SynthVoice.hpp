#pragma once

#include "MPEVoice.hpp"
#include "connectiongraph/module.hpp"
#include "connectiongraph/connectiongraph.hpp"

namespace PhasePhckr {

class SynthVoiceI {
public:
    MPEVoice mpe;
    virtual void reset() = 0;
    virtual void update(float * buffer, int numSamples, float sampleRate) = 0;
};

// special modules for the bus
class InputBusModule : public Module {
    // hooks up all the voice+global inputs and outputs
public:
    InputBusModule() {
        outputs.push_back(Pad("Gate"));
        outputs.push_back(Pad("StrikeZ"));
        outputs.push_back(Pad("LiftZ"));
        outputs.push_back(Pad("PitchHz"));
        outputs.push_back(Pad("GlideX"));
        outputs.push_back(Pad("SlideY"));
        outputs.push_back(Pad("PressZ"));
    }

    void updateVoice(const MPEVoiceState &state) {
        outputs[0].value = state.gate;
        outputs[1].value = state.strikeZ;
        outputs[2].value = state.liftZ;
        outputs[3].value = state.pitchHz;
        outputs[4].value = state.glideX;
        outputs[5].value = state.slideY;
        outputs[6].value = state.pressZ;
    }

    virtual void process(uint32_t fs) {};

};

class OutputBusModule : public Module {
public:
    OutputBusModule() {
        inputs.push_back(Pad("MonoOut"));
        outputs.push_back(Pad("MonoOut"));
    }
    virtual void process(uint32_t fs) {
        outputs[0].value = inputs[0].value;
    };
};

class ExConnectionGraphVoice : public SynthVoiceI {
public:
    ExConnectionGraphVoice() :
        connectionGraph(48000),
        t(0),
        rms(0),
        rmsSlew(0.999)
    {
        // example patch to toy with the requirements on routing

        inBus = connectionGraph.addModule(new InputBusModule());
        outBus = connectionGraph.addModule(new OutputBusModule());

        int phase = connectionGraph.addModule("PHASE");
        int mixGain = connectionGraph.addModule("MUL");
        int env = connectionGraph.addModule("ENV");

        connectionGraph.connect(inBus, 0, env, 0);
        connectionGraph.connect(inBus, 1, env, 1);
        connectionGraph.connect(inBus, 2, env, 5);
        connectionGraph.connect(inBus, 6, env, 4);
        connectionGraph.connect(inBus, 3, phase, 0);

        // osc 1, f0 cohesion...
        int osc1 = connectionGraph.addModule("SINE");
        connectionGraph.connect(phase, 0, osc1, 0);
        connectionGraph.connect(osc1, 0, mixGain, 0);

        // osc 2 and osc 3, crossfade on Y and atan prescale on Z (... env)
        int inv = connectionGraph.addModule("CINV"); // inverses inside the bounds
        int osc2arg = connectionGraph.addModule("SATAN");
        connectionGraph.connect(phase, 0, osc2arg, 0);
        int osc2argBoost = connectionGraph.addModule("MUL");
        connectionGraph.connect(env, 0, inv, 0); // flip it, for educational purposes
        connectionGraph.connect(inv, 0, osc2argBoost, 0);
        connectionGraph.getModule(osc2argBoost)->setFloatingValue(1, 20); // cheat?
        connectionGraph.connect(osc2argBoost, 0, osc2arg, 1);
        int osc2 = connectionGraph.addModule("SINE");
        connectionGraph.connect(osc2arg, 0, osc2, 0);

        int osc3arg = connectionGraph.addModule("MUL");
        connectionGraph.connect(osc2arg, 0, osc3arg, 0);
        connectionGraph.getModule(osc3arg)->setFloatingValue(1, 2); // cheat?
        int osc3 = connectionGraph.addModule("SINE");
        connectionGraph.connect(osc3arg, 0, osc3, 0);

        int osc23 = connectionGraph.addModule("XFADE");
        connectionGraph.connect(osc2, 0, osc23, 1);
        connectionGraph.connect(osc3, 0, osc23, 0);
        connectionGraph.connect(inBus, 5, osc23, 2);

        connectionGraph.connect(osc23, 0, mixGain, 0);

        // osc4 - wavefolded sine, prescale on Z (... env)
        int osc4 = connectionGraph.addModule("SINE");
        int fold = connectionGraph.addModule("FOLD");
        int foldScale = connectionGraph.addModule("MUL");
        connectionGraph.getModule(foldScale)->setFloatingValue(1, 5); // cheat?

        connectionGraph.connect(phase, 0, osc4, 0);
        connectionGraph.connect(foldScale, 0, fold, 4);
        connectionGraph.connect(env, 0, foldScale, 0);
        connectionGraph.connect(osc4, 0, fold, 0);
        connectionGraph.connect(fold, 0, mixGain, 0);

        // TODO try out SPOW also

        // mix amplitude on Z (... env)
        connectionGraph.connect(env, 0, mixGain, 1);
        connectionGraph.connect(mixGain, 0, outBus, 0);
    }

    virtual void reset() {}

    virtual void update(float * buffer, int numSamples, float sampleRate) {
        InputBusModule* inbusPtr = (InputBusModule*)connectionGraph.getModule(inBus);
        const MPEVoiceState &v = mpe.getState();
        if (v.gate == 0 && rms < 0.0000001) {
            mpe.update();
            t += numSamples;
            return;
        }
        for (int i = 0; i < numSamples; ++i) {
            inbusPtr->updateVoice(mpe.getState());
            mpe.update();
            connectionGraph.process(outBus, t + i);
            float sample = connectionGraph.getOutput(outBus, 0);
            buffer[i] += 0.5*sample;
            rms = rms*rmsSlew + (1 - rmsSlew)*(sample*sample); // without the root
        }
        t += numSamples;
    }

private:
    ConnectionGraph connectionGraph;
    unsigned long t;
    int inBus;
    int outBus;
    float rmsSlew;
    float rms;
};


}
