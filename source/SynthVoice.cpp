#include "SynthVoice.hpp"

namespace PhasePhckr
{

SynthVoice::SynthVoice()
    : connectionGraph()
    , t(0)
    , rms(0)
    , rmsSlew(0.999)
{
    connectionGraph.registerModule("VOICEINPUT", &VoiceInputBus::factory);
    connectionGraph.registerModule("STEREOBUS", &StereoBus::factory);
    ModuleRegister::registerAllModules(connectionGraph);
    // example patch to toy with the requirements on routing

    // input/output bus + "vca" + one envelope
    inBus = connectionGraph.addModule("VOICEINPUT");
    outBus = connectionGraph.addModule("STEREOBUS");
    int mixGain = connectionGraph.addModule("MUL");
    int env = connectionGraph.addModule("ENV");

    // hook up controller messages to envelope ... quite elaborate ;)

    connectionGraph.connect(inBus, 0, env, 0);
    connectionGraph.connect(inBus, 1, env, 1);
    connectionGraph.connect(inBus, 2, env, 5);
    connectionGraph.connect(inBus, 6, env, 4);

    // if pressing down on expression, velocity adds more and more to decay length
    // and aftertouch does less and less for sustain ...
    // effectively morphing into a velocity based pluck instrument
    int strikeMulExpr = connectionGraph.addModule("MUL");
    int exprInv = connectionGraph.addModule("CINV");
    connectionGraph.connect(inBus, 8, strikeMulExpr, 0);
    connectionGraph.connect(inBus, 1, strikeMulExpr, 1);
    int envDecayTweak = connectionGraph.addModule("SCLSHFT");
    connectionGraph.getModule(envDecayTweak)->setInput(1, 2.0);
    connectionGraph.getModule(envDecayTweak)->setInput(2, 0.1);
    connectionGraph.connect(strikeMulExpr, 0, envDecayTweak, 0);
    connectionGraph.connect(envDecayTweak, 0, env, 3);

    // oscillators ...
    int phase = connectionGraph.addModule("PHASE");
    connectionGraph.connect(inBus, 3, phase, 0);

    // osc 2 and osc 3, crossfade on Y and atan prescale on Z (... env) - Z direction depends on modwheel position!
    int ySelection = connectionGraph.addModule("XFADE");
    int inv = connectionGraph.addModule("CINV"); // inverses inside the bounds
    int osc2arg = connectionGraph.addModule("SATAN");
    connectionGraph.connect(phase, 0, osc2arg, 0);
    int osc2argBoost = connectionGraph.addModule("MUL");
    connectionGraph.connect(env, 0, inv, 0);
    connectionGraph.connect(env, 0, ySelection, 0);
    connectionGraph.connect(inv, 0, ySelection, 1);
    connectionGraph.connect(inBus, 7, ySelection, 2);
    connectionGraph.connect(ySelection, 0, osc2argBoost, 0);
    connectionGraph.getModule(osc2argBoost)->setInput(1, 20);
    connectionGraph.connect(osc2argBoost, 0, osc2arg, 1);
    int osc2 = connectionGraph.addModule("SINE");
    connectionGraph.connect(osc2arg, 0, osc2, 0);

    int osc3arg = connectionGraph.addModule("MUL");
    connectionGraph.connect(osc2arg, 0, osc3arg, 0);
    connectionGraph.getModule(osc3arg)->setInput(1, 2);
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
    connectionGraph.getModule(foldPreScale)->setInput(1, 4);
    connectionGraph.connect(foldPreScale, 0, fold, 2);
    connectionGraph.connect(env, 0, foldPreScale, 0);
    connectionGraph.connect(scl, 0, fold, 0);
    int lag = connectionGraph.addModule("LAG"); // simplistic lowpass
    connectionGraph.connect(fold, 0, lag, 0);
    int foldPostScale = connectionGraph.addModule("MUL");
    connectionGraph.getModule(foldPostScale)->setInput(1, 0.25);
    connectionGraph.connect(lag, 0, foldPostScale, 0);

    connectionGraph.connect(foldPostScale, 0, mixGain, 0);

    // TODO try out SPOW also

    // mix amplitude on Z (... env)
    connectionGraph.connect(env, 0, mixGain, 1);
    connectionGraph.connect(mixGain, 0, outBus, 0);
    connectionGraph.connect(mixGain, 0, outBus, 1);
}

void SynthVoice::update(float * bufferL, float * bufferR, int numSamples, float sampleRate, const GlobalData& globalData) {
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
        mpe.update();
        const MPEVoiceState &state = mpe.getState();
        connectionGraph.setInput(inBus, 0, state.gate);
        connectionGraph.setInput(inBus, 1, state.strikeZ);
        connectionGraph.setInput(inBus, 2, state.liftZ);
        connectionGraph.setInput(inBus, 3, state.pitchHz);
        connectionGraph.setInput(inBus, 4, state.glideX);
        connectionGraph.setInput(inBus, 5, state.slideY);
        connectionGraph.setInput(inBus, 6, state.pressZ);
        connectionGraph.setInput(inBus, 7, globalData.mod);
        connectionGraph.setInput(inBus, 8, globalData.exp);
        connectionGraph.setInput(inBus, 9, globalData.brt);

        connectionGraph.process(outBus, sampleRate);
        float sampleL = connectionGraph.getOutput(outBus, 0);
        float sampleR = connectionGraph.getOutput(outBus, 1);
        bufferL[i] += 0.5f*sampleL;
        bufferR[i] += 0.5f*sampleR;
        rms = rms*rmsSlew + (1 - rmsSlew)*((sampleL+sampleR)*(sampleL+sampleR)); // without the root
    }
    t += numSamples;
}

};

