#include "camelenvelope.hpp"
#include "inlines.hpp"

CamelEnvelope::CamelEnvelope()
{
    outputs.push_back(Pad("value"));

    inputs.push_back(Pad("gate"));

    inputs.push_back(Pad("onBumpHeight", 0.5f));
    inputs.push_back(Pad("onAttackSpeed", 0.025f));
    inputs.push_back(Pad("onDecaySpeed", 0.05f));

    inputs.push_back(Pad("sustainHeight", 0.5f));

    inputs.push_back(Pad("offBumpHeight", 0.05f));
    inputs.push_back(Pad("offAttackSpeed", 0.05f));
    inputs.push_back(Pad("offDecaySpeed", 0.25f));

    inputs.push_back(Pad("onAttackPow", 0.5f));
    inputs.push_back(Pad("onDecayPow", 2.0f));

    inputs.push_back(Pad("offAttackPow", 4.0f));
    inputs.push_back(Pad("offDecayPow", 4.0f));
}

static inline float limitValue(float value, float low, float high) {
    return value > high ? high : value < low ? low : value;
}

static inline float limitValueLow(float value, float low) {
    return value < low ? low : value;
}

inline void CamelEnvelope::changeState(EnvelopeStage newState) {
    if (newState == OnAttack) stageScale = onAttackSamples ? 1.f / onAttackSamples : stageScale;
    else if (newState == OnDecay) stageScale = onDecaySamples ? 1.f / onDecaySamples : stageScale;
    else if (newState == OffAttack) stageScale = offAttackSamples ? 1.f / offAttackSamples : stageScale;
    else if (newState == OffDecay) stageScale = offDecaySamples ? 1.f / offDecaySamples : stageScale;
    stage = newState;
    stageSamples = 0.f;
}

void CamelEnvelope::process(uint32_t fs) {
    float newGate = inputs[0].value;

    float f_fs = (float)fs;

    float onBumpHeight = limitValue(inputs[1].value, 0.0f, 1.0f);
    onAttackSamples = limitValueLow(inputs[2].value, 0.f) * f_fs;
    onDecaySamples = limitValueLow(inputs[3].value, 0.f) * f_fs;
    float sustainHeight = limitValue(inputs[4].value, 0.0f, 1.0f);
    float offBumpHeight = limitValue(inputs[5].value, 0.0f, 1.0f);
    offAttackSamples = limitValueLow(inputs[6].value, 0.f) * f_fs;
    offDecaySamples = limitValueLow(inputs[7].value, 0.f) * f_fs;

    float onAttackPow = limitValueLow(inputs[8].value, 0.f);
    float onDecayPow = limitValueLow(inputs[9].value, 0.f);

    float offAttackPow = limitValueLow(inputs[10].value, 0.f);
    float offDecayPow = limitValueLow(inputs[11].value, 0.f);

    float targetValue = 0.f;

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

    value = slew*value + (1 - slew)*targetValue;

    // limit, apply power law and slew
    outputs[0].value = value;

}
