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
    float currPow = 1.0;
    float nextPow = 1.0;
    float relTime = 0.0;

    float envTime = (float)samplesCtr / (float) fs;
    if(gate){
        if(envTime < onAttackSpeed){
            // attack region
            relTime = envTime / onAttackSpeed;
            targetValue = (sustainHeight + onBumpHeight) * relTime;
            currPow = onAttackPow;
            nextPow = onDecayPow;
        }
        else if(envTime < (onAttackSpeed + onDecaySpeed)){
            // decay region
            relTime = (envTime - onAttackSpeed) / onDecaySpeed;
            targetValue = sustainHeight + onBumpHeight * (1-relTime);
            currPow = onDecayPow;
            nextPow = 1;
        }
        else{
            // sustain region
            relTime = 1.0f; // egal, actually
            targetValue = sustainHeight;
            currPow = 1.f;
            nextPow = 1.f;
        }
        gateOnTargetValue = targetValue; // so release has a reference
    }
    else{
        if(envTime < offAttackSpeed){
            // release attack region
            relTime = envTime / offAttackSpeed;
            targetValue = gateOnTargetValue + offBumpHeight * relTime;
            currPow = offAttackPow;
            nextPow = offDecayPow;
        }
        else if(envTime < (offAttackSpeed + offDecaySpeed)){
            // release decay region
            relTime = (envTime - offAttackSpeed) / offDecaySpeed;
            targetValue = (gateOnTargetValue + offBumpHeight) * (1-relTime);
            currPow = offDecayPow;
            nextPow = 1.f;
        }
        else{
            // closed region
            relTime = 0.f; // egal
            targetValue = 0;
            gateOnTargetValue = 0;
            currPow = 1.f;
            nextPow = 1.f;
        }
    }

    samplesCtr++;

    float fadePow = currPow * (1-relTime) + nextPow * relTime; // linear trend of pow

    // limit, apply power law and slew
    targetValue = targetValue > 1.f ? 1.f : targetValue < 0.f ? 0.f : targetValue;
    outputs[0].value = slew*outputs[0].value + (1-slew)*powf(targetValue, fadePow);
}
