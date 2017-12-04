#ifndef CAMELENVELOPE_HPP
#define CAMELENVELOPE_HPP

#include "module.hpp"

class CamelEnvelope : public ModuleCRTP<CamelEnvelope>
{
public:
    CamelEnvelope();
    void process(uint32_t fs);
    static Module* factory() { return new CamelEnvelope(); }
    virtual std::string docString() { return "Trigger attack/release stages on gate flank. Can behave like a traditional ADSR, but also has a 'release' bump. The shapes are controllable from convex, through linear to concave via a power law."; };
    enum EnvelopeStage {
        Idle,
        OnAttack,
        OnDecay,
        Sustain,
        OffAttack,
        OffDecay
    };
private:
    float value = 0.f;
    float gate = 0.f;
    float gateOnTargetValue = 0.f;
    const float slew = 0.9f;
    float stageSamples = 0.f;
    inline void changeState(EnvelopeStage newState);
    EnvelopeStage stage = Idle;
    float stageScale = 1.f;
    float onAttackSamples = 0.f;
    float onDecaySamples = 0.f;
    float offAttackSamples = 0.f;
    float offDecaySamples = 0.f;
};

#endif
