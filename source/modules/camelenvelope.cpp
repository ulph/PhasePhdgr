#include "camelenvelope.hpp"

CamelEnvelope::CamelEnvelope():
    gate(0),
    gateOnTargetValue(0),
    slew(0.9f),
    samplesCtr(0)
{
    outputs.push_back(Pad("envelopeValue"));

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

static float limitValue(float value, float low, float high) {
    return value > high ? high : value < low ? low : value;
}

void CamelEnvelope::process(uint32_t fs) {
    float newGate       = inputs[0].value;

    float onBumpHeight   = limitValue(inputs[1].value, 0.0f, 1.0f);
    float onAttackSpeed  = inputs[2].value;
    float onDecaySpeed   = inputs[3].value;

    float sustainHeight  = limitValue(inputs[4].value, 0.0f, 1.0f);

    float offBumpHeight  = limitValue(inputs[5].value, 0.0f, 1.0f);
    float offAttackSpeed = inputs[6].value;
    float offDecaySpeed  = inputs[7].value;

    float onAttackPow    = inputs[8].value;
    float onDecayPow     = inputs[9].value;

    float offAttackPow   = inputs[10].value;
    float offDecayPow    = inputs[11].value;

    float targetValue = 0;

    // no targets can be greater than 1
    onBumpHeight = (sustainHeight + onBumpHeight) > 1.0f ? 1.0f - sustainHeight : onBumpHeight;
    offBumpHeight = (sustainHeight + offBumpHeight) > 1.0f ? 1.0f - sustainHeight : offBumpHeight;

    // reset counter if falling or rising edge on gate
    if( (gate >= 1.0 && newGate < 1.0) || (gate < 1.0 && newGate >= 1.0) ){
        samplesCtr = 0;
    }
    gate = newGate;

    float envTime = (float)samplesCtr / (float) fs;
    float relTime = 0;
    if(gate){
        if(envTime < onAttackSpeed){
            // attack region
            relTime = envTime / onAttackSpeed;
            targetValue = (sustainHeight + onBumpHeight) * powf(relTime, onAttackPow);
        }
        else if(envTime < (onAttackSpeed + onDecaySpeed)){
            // decay region
            relTime = (envTime - onAttackSpeed) / onDecaySpeed;
            targetValue = sustainHeight + onBumpHeight * powf((1-relTime), onDecayPow);
        }
        else{
            // sustain region
            relTime = 1.0f; // egal, actually
            targetValue = sustainHeight;
        }
        gateOnTargetValue = targetValue; // so release has a reference
    }
    else{
        if(envTime < offAttackSpeed){
            // release attack region
            relTime = envTime / offAttackSpeed;
            targetValue = gateOnTargetValue + offBumpHeight * powf(relTime, offAttackPow);
        }
        else if(envTime < (offAttackSpeed + offDecaySpeed)){
            // release decay region
            relTime = (envTime - offAttackSpeed) / offDecaySpeed;
            targetValue = (gateOnTargetValue + offBumpHeight) * powf((1-relTime), offDecayPow);
        }
        else{
            // closed region
            relTime = 0.f; // egal
            targetValue = 0;
            gateOnTargetValue = 0;
        }
    }

    samplesCtr++;

    // limit, apply power law and slew
    outputs[0].value = slew*outputs[0].value + (1-slew)*targetValue;
}
