#ifndef CAMELENVELOPE_HPP
#define CAMELENVELOPE_HPP

#include "module.hpp"
#include "rlc.hpp"

class CamelEnvelope : public ModuleCRTP<CamelEnvelope>
{
public:
    CamelEnvelope();
    void processSample(int sample) override;
    static Module* factory() { return new CamelEnvelope(); }
    virtual std::string docString() const override {
        return "Trigger attack/release stages on gate flank. Can behave like a traditional ADSR, but also has a 'release' bump. The shapes are controllable from convex, through linear to concave via a power law."; 
    };
    enum EnvelopeStage {
        Idle,
        OnAttack,
        OnDecay,
        Sustain,
        OffAttack,
        OffDecay
    };
protected:
    virtual void init() override;
private:
    float targetValue = 0.f;
    float value = 0.f;
    float gate = 0.f;
    float gateOnTargetValue = 0.f;
    float slew = 0.9f;
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
