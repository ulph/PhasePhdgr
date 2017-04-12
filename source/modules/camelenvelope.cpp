#include "camelenvelope.hpp"

CamelEnvelope::CamelEnvelope():
    gate(0),
    gateOnTargetValue(0),
    slew(0.9f),
    samplesCtr(0)
{
    outputs.push_back(Pad("envelopeValue"));

    inputs.push_back(Pad("gate"));
    inputs.push_back(Pad("onBumpHeight", 1.0f));
    inputs.push_back(Pad("onAttackSpeed", 0.01f));
    inputs.push_back(Pad("onDecaySpeed", 0.05f));
    inputs.push_back(Pad("sustainHeight", 0.5f));
    inputs.push_back(Pad("offBumpHeight", 0.25f));
    inputs.push_back(Pad("offAttackSpeed", 0.05f));
    inputs.push_back(Pad("offDecaySpeed", 0.05f));

    // power laws for snappyness
    inputs.push_back(Pad("onAttackPow", 2.0f));
    inputs.push_back(Pad("onDecayPow", 0.5f));
    inputs.push_back(Pad("offAttackPow", 2.0f));
    inputs.push_back(Pad("offDecayPow", 0.5f));

}

void CamelEnvelope::process(uint32_t fs) {
    float newGate       = inputs[0].value;
    float onBumpHeight   = inputs[1].value;
    float onAttackSpeed  = inputs[2].value;
    float onDecaySpeed   = inputs[3].value;
    float sustainHeight  = inputs[4].value;
    float offBumpHeight  = inputs[5].value;
    float offAttackSpeed = inputs[6].value;
    float offDecaySpeed  = inputs[7].value;

    float onAttackPow    = inputs[8].value;
    float onDecayPow     = inputs[9].value;
    float offAttackPow   = inputs[10].value;
    float offDecayPow    = inputs[11].value;

    float targetValue = 0;

    // reset counter if falling or rising edge on gate
    if( (gate >= 1.0 && newGate < 1.0) || (gate < 1.0 && newGate >= 1.0) ){
        samplesCtr = 0;
    }
    gate = newGate;
    float pow = 1.0;

    float envTime = (float)samplesCtr / (float) fs;
    if(gate){
        if(envTime < onAttackSpeed){
            // attack region
            targetValue = (sustainHeight + onBumpHeight) * (envTime / onAttackSpeed);
            pow = onAttackPow;
        }
        else if(envTime < (onAttackSpeed + onDecaySpeed)){
            // decay region
            targetValue = sustainHeight + onBumpHeight * (1 - ((envTime - onAttackSpeed) / onDecaySpeed));
            pow = onDecayPow;
        }
        else{
            // sustain region
            targetValue = sustainHeight;
            pow = 1.f;
        }
        gateOnTargetValue = targetValue; // so release has a reference
    }
    else{
        if(envTime < offAttackSpeed){
            // release attack region
            targetValue = gateOnTargetValue + offBumpHeight * (envTime / offAttackSpeed);
            pow = offAttackPow;
        }
        else if(envTime < (offAttackSpeed + offDecaySpeed)){
            // release decay region
            targetValue = (gateOnTargetValue + offBumpHeight) * (1 - ((envTime - offAttackSpeed) / offDecaySpeed));
            pow = offDecayPow;
        }
        else{
            // closed region
            targetValue = 0;
            gateOnTargetValue = 0;
            pow = 1.f;
        }
    }

    samplesCtr++;

    // limit and slew
    targetValue = targetValue > 1.f ? 1.f : targetValue < 0.f ? 0.f : targetValue;
    outputs[0].value = slew*outputs[0].value + (1-slew)*powf(targetValue, pow);
}
