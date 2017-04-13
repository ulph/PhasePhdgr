#ifndef MODULEREGISTER_HPP
#define MODULEREGISTER_HPP

#include "connectiongraph.hpp"
#include "phase.hpp"
#include "square.hpp"
#include "mul.hpp"
#include "clamp.hpp"
#include "quant8.hpp"
#include "noise.hpp"
#include "sine.hpp"
#include "saturatoratan.hpp"
#include "camelenvelope.hpp"
#include "lag.hpp"
#include "abs.hpp"
#include "foldback.hpp"
#include "crossfade.hpp"
#include "sympow.hpp"
#include "clampinv.hpp"
#include "scaleshift.hpp"
#include "delay.hpp"
#include "biquad.hpp"

class ModuleRegister {
public:
    static void registerAllModules(ConnectionGraph &cg)
    {
        cg.registerModule("PHASE", &(Phase::factory));
        cg.registerModule("SQUARE", &(Square::factory));
        cg.registerModule("MUL", &(Mul::factory));
        cg.registerModule("GAIN", &(Gain::factory));
        cg.registerModule("CLAMP", &(Clamp::factory));
        cg.registerModule("QUANT8", &(Quant8::factory));
        cg.registerModule("NOISE", &(Noise::factory));
        cg.registerModule("SINE", &(Sine::factory));
        cg.registerModule("SATAN", &(SaturatorAtan::factory));
        cg.registerModule("SSATAN", &(StereoSaturatorAtan::factory));
        cg.registerModule("ENV", &(CamelEnvelope::factory));
        cg.registerModule("LAG", &(Lag::factory)); // TODO, prune
        cg.registerModule("RCHP", &(RcHp::factory));
        cg.registerModule("RCLP", &(RcLp::factory));
        cg.registerModule("ABS", &(Abs::factory));
        cg.registerModule("FOLD", &(FoldBack::factory));
        cg.registerModule("XFADE", &(CrossFade::factory));
        cg.registerModule("SPOW", &(SymPow::factory));
        cg.registerModule("CINV", &(ClampInv::factory));
        cg.registerModule("SCLSHFT", &(ScaleShift::factory));
        cg.registerModule("DELAY", &(Delay::factory));
        cg.registerModule("BIQUAD", &(Biquad::factory));
        cg.registerModule("LPF", &(LowPass::factory));
    }
};

#endif
