#ifndef MODULEREGISTER_HPP
#define MODULEREGISTER_HPP

#include "connectiongraph.hpp"
#include "phase.hpp"
#include "mul.hpp"
#include "quantize.hpp"
#include "noise.hpp"
#include "sine.hpp"
#include "env.hpp"
#include "rlc.hpp"
#include "foldback.hpp"
#include "crossfade.hpp"
#include "sympow.hpp"
#include "map.hpp"
#include "delay.hpp"
#include "biquad.hpp"
#include "blitosc.hpp"
#include "svf.hpp"
#include "conversion.hpp"
#include "samphold.hpp"
#include "div.hpp"
#include "logic.hpp"
#include "zdf.hpp"
#include "unarymath.hpp"

class Constant : public ModuleCRTP<Constant>
{
public:
    Constant() {
        inputs.push_back(Pad("value"));
        outputs.push_back(Pad("value"));
    }
    void process() {
        outputs[0].value = inputs[0].value;
    }
    virtual std::string docString() { return "A 'constant' of questionable value."; }
    static Module* factory() { return new Constant(); }
};


class Knob : public ModuleCRTP<Knob> {
    public:
    Knob() {
        inputs.push_back(Pad("value"));
        inputs.push_back(Pad("min"));
        inputs.push_back(Pad("max"));
        outputs.push_back(Pad("value"));
    }
    void process() {
        outputs[0].value = inputs[0].value;
    }
    static Module* factory() { return new Knob(); }
    virtual std::string docString() { return "A knob representing an automatable parameter. It will show up in the HOST as well as the parameters tab."; }
};


class ModuleRegister {
public:
    static void registerAllModules(ConnectionGraph &cg)
    {
        /* mul/div */
        cg.registerModule("MUL", &(Mul::factory));
        cg.registerModule("MULTRI", &(MulTri::factory));
        cg.registerModule("MULQUAD", &(MulQuad::factory));
        cg.registerModule("DIV", &(Div::factory));
        cg.registerModule("MOD", &(Mod::factory));
        cg.registerModule("ABS", &(Abs::factory));

        /* map */
        cg.registerModule("MAP", &(RangeMap::factory)); // hmm
        cg.registerModule("CLAMP", &(Clamp::factory));

        cg.registerModule("CONST", &(Constant::factory));
        
        /* mixing */
        cg.registerModule("XFADE", &(CrossFade::factory));
        cg.registerModule("FADEX", &(FadeCross::factory));
        // TODO, XFADE / FADEX variants with more io, aka (DE)MUX

        /* logic */
        cg.registerModule("SAMPHOLD", &(SampleAndHold::factory));
        cg.registerModule("TRESH", &(Threshold::factory));
        cg.registerModule("COUNTER", &(Counter::factory));
        // TODO; AND/OR/XOR ... NEG etc

        /* conversions */
        cg.registerModule("TEMPO2TIME", &(TempoToTime::factory));
        cg.registerModule("TRANSPOSE", &(Transpose::factory));

        /* generators */
        cg.registerModule("PHASE", &(Phase::factory));
        cg.registerModule("NOISE", &(Noise::factory));
        cg.registerModule("OSC_BLIT", &(BlitOsc::factory));

        /* filters */
        cg.registerModule("BQ_FILTER", &(Biquad::factory));
        cg.registerModule("BQ_LPF", &(LowPass::factory));
        cg.registerModule("BQ_PEAK", &(PeakingEQ::factory));

        cg.registerModule("ZDF_SVF", &(TrapezoidalTanSVF::factory));
        cg.registerModule("ZDF_OSVF", &(OpenTrapezoidalTanSVF::factory));

        cg.registerModule("ZDF_1P", &(Zdf1p::factory));
        cg.registerModule("ZDF_1PLSHELF", &(Zdf1pLowShelf::factory));
        cg.registerModule("ZDF_1PHSHELF", &(Zdf1pHighShelf::factory));

        cg.registerModule("1PHP", &(RcHp::factory)); // TODO, rename?
        cg.registerModule("1PLP", &(RcLp::factory)); // TODO, rename?

        /* special filters */
        cg.registerModule("LAG", &(Lag::factory));
        cg.registerModule("RATELIMITER", &(RateLimiter::factory)); // TODO, not a filter
        cg.registerModule("INTEGRATOR", &(LeakyIntegrator::factory)); // numerical leaky integrator

        /* envelopes */
        cg.registerModule("CAMELENV", &(CamelEnvelope::factory));
        // TODO ADSR-env

        /* buffers */
        cg.registerModule("UNITDELAY", UnitDelay::factory);
        cg.registerModule("DELAY", DelayFactory::makeFactory(32));
        cg.registerModule("DELAY_LOW_Q", DelayFactory::makeFactory(16));
        cg.registerModule("DELAY_LOWER_Q", DelayFactory::makeFactory(8));
        cg.registerModule("DELAY_LOWEST_Q", DelayFactory::makeFactory(4));
        // TODO, an actual record/playback buffer thingy with controllable play/record speed and/or position

        /* shaping */
        cg.registerModule("SINE", &(Sine::factory));
        cg.registerModule("SPOW", &(SymPow::factory));
        cg.registerModule("SLOG2", &(SymLog2::factory));
        cg.registerModule("QUANT", &(Quantize::factory));
        cg.registerModule("FOLD", &(FoldBack::factory));
        cg.registerModule("WRAP", &(Wrap::factory));
        // ... unary math operators ...
        cg.registerModule("ATAN", &(Atan::factory));
        cg.registerModule("SATAN", &(NormalizedAtan::factory));
        cg.registerModule("SSATAN", &(StereoNormalizedAtan::factory));
        cg.registerModule("TANH", &(TanH::factory));
        cg.registerModule("NTANH", &(NormalizedTanH::factory));
        cg.registerModule("SNTANH", &(StereoNormalizedTanH::factory));
        cg.registerModule("ASINH", &(ArcSinH::factory));
        cg.registerModule("NASINH", &(NormalizedArcSinH::factory));
        cg.registerModule("SNASINH", &(StereoNormalizedArcSinH::factory));

        auto a = asinhf(2);
        
        /* stereo */
        cg.registerModule("GAIN", &(Gain::factory));

        /* special */
        cg.registerModule("=KNOB", &(Knob::factory));

    }
};

#endif
