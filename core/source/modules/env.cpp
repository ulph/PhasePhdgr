#include "env.hpp"
#include "inlines.hpp"

CamelEnvelope::CamelEnvelope()
{
    outputs.push_back(Pad("out"));

    inputs.push_back(Pad("gate"));

    inputs.push_back(Pad("onBumpHeight", 0.5f));
    inputs.push_back(Pad("onAttackSpeed", 0.025f, "s"));
    inputs.push_back(Pad("onDecaySpeed", 0.05f, "s"));

    inputs.push_back(Pad("sustainHeight", 0.5f));

    inputs.push_back(Pad("offBumpHeight", 0.05f));
    inputs.push_back(Pad("offAttackSpeed", 0.05f, "s"));
    inputs.push_back(Pad("offDecaySpeed", 0.25f, "s"));

    inputs.push_back(Pad("onAttackPow", 0.5f));
    inputs.push_back(Pad("onDecayPow", 2.0f));

    inputs.push_back(Pad("offAttackPow", 4.0f));
    inputs.push_back(Pad("offDecayPow", 4.0f));

    init();
}

void CamelEnvelope::init()
{
    slew = DesignRcLp(100, fsInv); // TODO, allow tweaking ?
}

inline void CamelEnvelope::changeState(EnvelopeStage newState) {
    if (newState == OnAttack) stageScale = onAttackSamples ? 1.f / onAttackSamples : stageScale;
    else if (newState == OnDecay) stageScale = onDecaySamples ? 1.f / onDecaySamples : stageScale;
    else if (newState == OffAttack) stageScale = offAttackSamples ? 1.f / offAttackSamples : stageScale;
    else if (newState == OffDecay) stageScale = offDecaySamples ? 1.f / offDecaySamples : stageScale;
    stage = newState;
    stageSamples = 0.f;
}

void CamelEnvelope::process() {
    const float newGate = inputs[0].value;

    float onBumpHeight = limit(inputs[1].value, 0.0f, 1.0f);
    onAttackSamples = limitLow(inputs[2].value, 0.f) * fs;
    onDecaySamples = limitLow(inputs[3].value, 0.f) * fs;
    float sustainHeight = limit(inputs[4].value, 0.0f, 1.0f);
    float offBumpHeight = limit(inputs[5].value, 0.0f, 1.0f);
    offAttackSamples = limitLow(inputs[6].value, 0.f) * fs;
    offDecaySamples = limitLow(inputs[7].value, 0.f) * fs;

    float onAttackPow = limitLow(inputs[8].value, 0.f);
    float onDecayPow = limitLow(inputs[9].value, 0.f);

    float offAttackPow = limitLow(inputs[10].value, 0.f);
    float offDecayPow = limitLow(inputs[11].value, 0.f);

    // no targets can be greater than 1
    onBumpHeight = (sustainHeight + onBumpHeight) > 1.0f ? 1.0f - sustainHeight : onBumpHeight;
    float hangoverValue = (gateOnTargetValue + offBumpHeight) > 1.0 ? 1.0f - offBumpHeight : gateOnTargetValue;

    if (!isHigh(gate) && isHigh(newGate)) {
        changeState(OnAttack);
    }
    else if(isHigh(gate) && !isHigh(newGate)) {
        changeState(OffAttack);
    }
    gate = newGate;

    switch (stage) {

    case Idle:
        targetValue = 0.f;
        break;

    case OnAttack:
        if (stageSamples >= onAttackSamples) changeState(OnDecay);
        else targetValue = gateOnTargetValue = (sustainHeight + onBumpHeight) * powf(stageSamples * stageScale, onAttackPow);
        break;

    case OnDecay:
        if (stageSamples >= onDecaySamples) changeState(Sustain);
        else targetValue = gateOnTargetValue = sustainHeight + onBumpHeight * powf((1 - stageSamples * stageScale), onDecayPow);
        break;

    case Sustain:
        targetValue = gateOnTargetValue = sustainHeight;
        break;

    case OffAttack:
        if (stageSamples >= offAttackSamples) changeState(OffDecay);
        else targetValue = hangoverValue + offBumpHeight * powf(stageSamples * stageScale, offAttackPow);
        break;

    case OffDecay:
        if (stageSamples >= offDecaySamples) changeState(Idle);
        else targetValue = (hangoverValue + offBumpHeight) * powf((1 - stageSamples * stageScale), offDecayPow);
        break;

    default:
        changeState(Idle);
        break;
    }

    stageSamples++;

    value = slew*targetValue + (1 - slew)*value;

    // limit, apply power law and slew
    outputs[0].value = value;

}
