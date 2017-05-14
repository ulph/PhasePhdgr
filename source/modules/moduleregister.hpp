#ifndef MODULEREGISTER_HPP
#define MODULEREGISTER_HPP

#include "connectiongraph.hpp"
#include "phase.hpp"
#include "mul.hpp"
#include "clamp.hpp"
#include "quant8.hpp"
#include "noise.hpp"
#include "sine.hpp"
#include "saturatoratan.hpp"
#include "camelenvelope.hpp"
#include "rlc.hpp"
#include "abs.hpp"
#include "foldback.hpp"
#include "crossfade.hpp"
#include "sympow.hpp"
#include "clampinv.hpp"
#include "scaleshift.hpp"
#include "delay.hpp"
#include "biquad.hpp"
#include "bleposc.hpp"
#include "chamberlin.hpp"


class Constant : public Module
{
public:
    Constant() {
        inputs.push_back(Pad("value"));
        outputs.push_back(Pad("value"));
    }
    void process(uint32_t fs) {
        outputs[0].value = inputs[0].value;
    }
    virtual std::string docString() { return "A 'constant' of questionable value."; };
    static Module* factory() { return new Constant(); }
};


class ModuleRegister {
public:
    static void registerAllModules(ConnectionGraph &cg)
    {
        /* logic */
        cg.registerModule("MUL", &(Mul::factory));
        cg.registerModule("ABS", &(Abs::factory));
        cg.registerModule("CINV", &(ClampInv::factory));
        cg.registerModule("SCLSHFT", &(ScaleShift::factory));
        cg.registerModule("CLAMP", &(Clamp::factory));

        /* generators */
        cg.registerModule("PHASE", &(Phase::factory));
        cg.registerModule("NOISE", &(Noise::factory));

        /* oscillators */
        cg.registerModule("PBLOSC", &(BlitOsc::factory));
        cg.registerModule("SINE", &(Sine::factory));

        /* filters */
        cg.registerModule("BIQUAD", &(Biquad::factory));
        cg.registerModule("BQLPF", &(LowPass::factory));
        cg.registerModule("BQPEAK", &(PeakingEQ::factory));
        cg.registerModule("CHAMBFLT", &(ChamberlinFilter::factory));
        cg.registerModule("RCHP", &(RcHp::factory));
        cg.registerModule("RCLP", &(RcLp::factory));
        cg.registerModule("ORCHP", &(OpenRcHp::factory));
        cg.registerModule("ORCLP", &(OpenRcLp::factory));

        /* envelopes */
        cg.registerModule("ENV", &(CamelEnvelope::factory));

        /* buffers */
        cg.registerModule("DELAY", &(Delay::factory));

        /* shaping */
        cg.registerModule("SPOW", &(SymPow::factory));
        cg.registerModule("QUANT8", &(Quant8::factory));
        cg.registerModule("SATAN", &(SaturatorAtan::factory));
        cg.registerModule("FOLD", &(FoldBack::factory));

        /* stereo */
        cg.registerModule("GAIN", &(Gain::factory));
        cg.registerModule("SSATAN", &(StereoSaturatorAtan::factory));

        /* convinience */
        cg.registerModule("CONST", &(Constant::factory));
        cg.registerModule("XFADE", &(CrossFade::factory));

    }
};

#endif
