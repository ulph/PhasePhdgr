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
#include <initializer_list>

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

inline auto getAllBuiltinModules(){
    static std::vector<std::pair<std::string,  Module* (*)()>> builtinModules = {
        {"MUL", &(Mul::factory)},
        {"MUL_TRI", &(MulTri::factory)},
        {"MUL_QUAD", &(MulQuad::factory)},
        {"DIV", &(Div::factory)},
        {"MOD", &(Mod::factory)},
        {"ABS", &(Abs::factory)},
        {"GAIN", &(Gain::factory)},
        {"XFADE", &(CrossFade::factory)},
        {"FADEX", &(FadeCross::factory)},
        {"MUX4", &(Mux4::factory)},
        {"MUX8", &(Mux8::factory)},
        {"SAMPHOLD", &(SampleAndHold::factory)},
        {"TRESH", &(Threshold::factory)},
        {"COUNTER", &(Counter::factory)},
        {"TEMPO2TIME", &(TempoToTime::factory)},
        {"TRANSPOSE", &(Transpose::factory)},
        {"PHASE", &(Phase::factory)},
        {"NOISE", &(Noise::factory)},
        {"OSC_BLIT", &(BlitOsc::factory)},
        {"BQ_FILTER", &(Biquad::factory)},
        {"BQ_LP", &(LowPass::factory)},
        {"BQ_PEAK", &(PeakingEQ::factory)},
        {"ZDF_SVF", &(TrapezoidalTanSVF::factory)},
        {"ZDF_OSVF", &(OpenTrapezoidalTanSVF::factory)},
        {"ZDF_OSVF2", &(OpenTrapezoidalTanSVF2::factory)},
        {"ZDF_1P", &(Zdf1p::factory)},
        {"ZDF_1PLSHELF", &(Zdf1pLowShelf::factory)},
        {"ZDF_1PHSHELF", &(Zdf1pHighShelf::factory)},
        {"ZDF_4PLADDER", &(Zdf4pLadder::factory)},
        {"D_HP", &(RcHp::factory)}, // TODO, rename?
        {"D_LP", &(RcLp::factory)}, // TODO, rename?
        {"LAG", &(Lag::factory)},
        {"RATELIMITER", &(RateLimiter::factory)}, // TODO, not a filter
        {"INTEGRATOR", &(LeakyIntegrator::factory)}, // numerical leaky integrator
        {"CAMELENV", &(CamelEnvelope::factory)},
        {"UNITDELAY", UnitDelay::factory},
        {"DELAY", DelayFactory::makeFactory(32)},
        {"DELAY_LOW_Q", DelayFactory::makeFactory(16)},
        {"DELAY_LOWER_Q", DelayFactory::makeFactory(8)},
        {"DELAY_LOWEST_Q", DelayFactory::makeFactory(4)},
        {"MAP", &(RangeMap::factory)},
        {"MULADD", &(ScaleShift::factory)},
        {"CLAMPINV", &(ClampInv::factory)},
        {"CLAMP", &(Clamp::factory)},
        {"SINE", &(Sine::factory)},
        {"SPOW", &(SymPow::factory)},
        {"SLOG2", &(SymLog2::factory)},
        {"QUANT", &(Quantize::factory)},
        {"FOLD", &(FoldBack::factory)},
        {"WRAP", &(Wrap::factory)},
        {"ATAN", &(Atan::factory)},
        {"NATAN", &(NormalizedAtan::factory)},
        {"SNATAN", &(StereoNormalizedAtan::factory)},
        {"TANH", &(TanH::factory)},
        {"NTANH", &(NormalizedTanH::factory)},
        {"SNTANH", &(StereoNormalizedTanH::factory)},
        {"ASINH", &(ArcSinH::factory)},
        {"NASINH", &(NormalizedArcSinH::factory)},
        {"SNASINH", &(StereoNormalizedArcSinH::factory)},
        {"CONST", &(Constant::factory)},
        {"=KNOB", &(Knob::factory)},
    };    
    return builtinModules;
}

class ModuleRegister {
public:
    static void registerAllModules(ConnectionGraph &cg)
    {
        const auto& builtinModules = getAllBuiltinModules();
        std::for_each(builtinModules.begin(), builtinModules.end(), [&cg](const auto kv){
           cg.registerModule(kv.first, kv.second); 
        });
    }
};

#endif
