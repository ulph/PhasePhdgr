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

class ExConnectionGraphVoice : public SynthVoiceI {

public:
    ConnectionGraphVoice() :
        connectionGraph(48000),
        t(0),
        rms(0),
        rmsSlew(0.999)
    {
        // example patch to toy with the requirements on routing

        inBus = connectionGraph.addModule("INPUT");
        outBus = connectionGraph.addModule("OUTPUT");

        int phase = connectionGraph.addModule("PHASE");
        int mixGain = connectionGraph.addModule("MUL");
        int env = connectionGraph.addModule("ENV");

        connectionGraph.connect(inBus, 0, env, 0);
        connectionGraph.connect(inBus, 1, env, 1);
        connectionGraph.connect(inBus, 2, env, 5);
        connectionGraph.connect(inBus, 6, env, 4);
        connectionGraph.connect(inBus, 3, phase, 0);

        // osc 2 and osc 3, crossfade on Y and atan prescale on Z (... env)
        int inv = connectionGraph.addModule("CINV"); // inverses inside the bounds
        int osc2arg = connectionGraph.addModule("SATAN");
        connectionGraph.connect(phase, 0, osc2arg, 0);
        int osc2argBoost = connectionGraph.addModule("MUL");
        connectionGraph.connect(env, 0, inv, 0); // flip it, for educational purposes
        connectionGraph.connect(inv, 0, osc2argBoost, 0);
        connectionGraph.getModule(osc2argBoost)->setInput(1, 20); // cheat?
        connectionGraph.connect(osc2argBoost, 0, osc2arg, 1);
        int osc2 = connectionGraph.addModule("SINE");
        connectionGraph.connect(osc2arg, 0, osc2, 0);

        int osc3arg = connectionGraph.addModule("MUL");
        connectionGraph.connect(osc2arg, 0, osc3arg, 0);
        connectionGraph.getModule(osc3arg)->setInput(1, 2); // cheat?
        int osc3 = connectionGraph.addModule("SINE");
        connectionGraph.connect(osc3arg, 0, osc3, 0);

        int osc23 = connectionGraph.addModule("XFADE");
        connectionGraph.connect(osc2, 0, osc23, 1);
        connectionGraph.connect(osc3, 0, osc23, 0);
        connectionGraph.connect(inBus, 5, osc23, 2);

        connectionGraph.connect(osc23, 0, mixGain, 0);

        // wavefolded triangle, prescale on Z (... env)
        int abs = connectionGraph.addModule("ABS");
        connectionGraph.connect(phase, 0, abs, 0);
        int scl = connectionGraph.addModule("SCLSHFT");
        connectionGraph.connect(abs, 0, scl, 0);
        int fold = connectionGraph.addModule("FOLD");
        int foldPreScale = connectionGraph.addModule("MUL");
        connectionGraph.getModule(foldPreScale)->setInput(1, 4); // cheat?
        connectionGraph.connect(foldPreScale, 0, fold, 2);
        connectionGraph.connect(env, 0, foldPreScale, 0);
        connectionGraph.connect(scl, 0, fold, 0);
        int lag = connectionGraph.addModule("LAG"); // simplistic lowpass
        connectionGraph.connect(fold, 0, lag, 0);
        int foldPostScale = connectionGraph.addModule("MUL");
        connectionGraph.getModule(foldPostScale)->setInput(1, 1); // cheat?
        connectionGraph.connect(lag, 0, foldPostScale, 0);

        connectionGraph.connect(foldPostScale, 0, mixGain, 0);

        // TODO try out SPOW also

        // mix amplitude on Z (... env)
        connectionGraph.connect(env, 0, mixGain, 1);
        connectionGraph.connect(mixGain, 0, outBus, 0);
    }

    virtual void reset() {}

    virtual void update(float * buffer, int numSamples, float sampleRate) {
        const MPEVoiceState &v = mpe.getState();
        if (v.gate) {
            rms = 1;
        }
        else if (v.gate == 0 && rms < 0.0000001) {
            mpe.update();
            t += numSamples;
            return;
        }
        for (int i = 0; i < numSamples; ++i) {
            const MPEVoiceState &state = mpe.getState();
            connectionGraph.setInput(inBus, 0, state.gate);
            connectionGraph.setInput(inBus, 1, state.strikeZ);
            connectionGraph.setInput(inBus, 2, state.liftZ);
            connectionGraph.setInput(inBus, 3, state.pitchHz);
            connectionGraph.setInput(inBus, 4, state.glideX);
            connectionGraph.setInput(inBus, 5, state.slideY);
            connectionGraph.setInput(inBus, 6, state.pressZ);
            mpe.update();
            connectionGraph.process(outBus);
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
