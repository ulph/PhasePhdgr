#ifndef MODULEREGISTER_HPP
#define MODULEREGISTER_HPP

#include "connectiongraph.hpp"
#include "phase.hpp"
#include "mul.hpp"
#include "clamp.hpp"
#include "quantize.hpp"
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
#include "blitosc.hpp"
#include "chamberlin.hpp"
#include "conversion.hpp"
#include "samphold.hpp"

class Constant : public ModuleCRTP<Constant>
{
public:
    Constant() {
        inputs.push_back(Pad("value"));
        outputs.push_back(Pad("value"));
    }
    void process(uint32_t fs) {
        outputs[0].value = inputs[0].value;
    }
    virtual std::string docString() { return "A 'constant' of questionable value."; }
    static Module* factory() { return new Constant(); }
};


class Knob : public ModuleCRTP<Knob> {
    public:
    Knob() {
        inputs.push_back(Pad("value"));
        outputs.push_back(Pad("value"));
    }
    void process(uint32_t fs) {
        outputs[0].value = inputs[0].value;
    }
    static Module* factory() { return new Knob(); }
    virtual PhasePhckr::ModuleDoc makeDoc() {
        auto d = Module::makeDoc();
        d.inputs.clear();
        return d;
    }
    virtual std::string docString() { return "A knob representing an automatable parameter. It will show up in the HOST as well as the parameters tab."; }
};


class ModuleRegister {
public:
    static void registerAllModules(ConnectionGraph &cg)
    {
        /* logic */
        cg.registerModule("MUL", &(Mul::factory));
        cg.registerModule("MULTRI", &(MulTri::factory));
        cg.registerModule("MULQUAD", &(MulQuad::factory));
        cg.registerModule("ABS", &(Abs::factory));
        cg.registerModule("CINV", &(ClampInv::factory));
        cg.registerModule("SCLSHFT", &(ScaleShift::factory));
        cg.registerModule("CLAMP", &(Clamp::factory));
        cg.registerModule("CONST", &(Constant::factory));
        cg.registerModule("XFADE", &(CrossFade::factory));
        cg.registerModule("SAMPHOLD", &(SampleAndHold::factory));
        // todo: counters, sample-and-hold, etc

        /* conversions */
        cg.registerModule("TEMPO2TIME", &(TempoToTime::factory));
        cg.registerModule("TRANSPOSE", &(Transpose::factory));

        /* generators */
        cg.registerModule("PHASE", &(Phase::factory));
        cg.registerModule("NOISE", &(Noise::factory));
        cg.registerModule("PBLOSC", &(BlitOsc::factory)); // DEPRECATED
        cg.registerModule("OSC_BLIT", &(BlitOsc::factory));

        /* filters */
        cg.registerModule("BIQUAD", &(Biquad::factory));
        cg.registerModule("BQLPF", &(LowPass::factory));
        cg.registerModule("BQPEAK", &(PeakingEQ::factory));
        cg.registerModule("CHAMBFLT", &(ChamberlinFilter::factory));
        cg.registerModule("OCHAMBFLT", &(OpenChamberlinFilter::factory));
        cg.registerModule("RCHP", &(RcHp::factory));
        cg.registerModule("RCLP", &(RcLp::factory));
        cg.registerModule("ORCHP", &(OpenRcHp::factory));
        cg.registerModule("ORCLP", &(OpenRcLp::factory));

        /* envelopes */
        cg.registerModule("ENV", &(CamelEnvelope::factory));

        /* buffers */
        cg.registerModule("DELAY", DelayFactory::makeFactory(32));
        cg.registerModule("DELAY_LOW_Q", DelayFactory::makeFactory(16));
        cg.registerModule("DELAY_LOWER_Q", DelayFactory::makeFactory(8));
        cg.registerModule("DELAY_LOWEST_Q", DelayFactory::makeFactory(4));

        /* shaping */
        cg.registerModule("SINE", &(Sine::factory));
        cg.registerModule("SPOW", &(SymPow::factory));
        cg.registerModule("QUANT", &(Quantize::factory));
        cg.registerModule("SATAN", &(SaturatorAtan::factory));
        cg.registerModule("FOLD", &(FoldBack::factory));

        /* stereo */
        cg.registerModule("GAIN", &(Gain::factory));
        cg.registerModule("SSATAN", &(StereoSaturatorAtan::factory));

        /* special */
        cg.registerModule("=KNOB", &(Knob::factory));

    }
};

#endif
