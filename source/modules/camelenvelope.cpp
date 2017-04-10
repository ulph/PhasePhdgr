#include "camelenvelope.hpp"

CamelEnvelope::CamelEnvelope():
    gate(0),
    target_value(0),
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
}

void CamelEnvelope::process(uint32_t fs) {
    float new_gate       = inputs[0].value;
    float onBumpHeight   = inputs[1].value;
    float onAttackSpeed  = inputs[2].value;
    float onDecaySpeed   = inputs[3].value;
    float sustainHeight  = inputs[4].value;
    float offBumpHeight  = inputs[5].value;
    float offAttackSpeed = inputs[6].value;
    float offDecaySpeed  = inputs[7].value;

    float target_value = 0;

    // reset counter if falling or rising edge on gate
    if( (gate >= 1.0 && new_gate < 1.0) || (gate < 1.0 && new_gate >= 1.0) ){
        samplesCtr = 0;
    }
    gate = new_gate;

    float envTime = (float)samplesCtr / (float) fs;
    if(gate){
        if(envTime < onAttackSpeed){
            // attack region
            target_value = (sustainHeight + onBumpHeight) * (envTime / onAttackSpeed);
        }
        else if(envTime < (onAttackSpeed + onDecaySpeed)){
            // decay region
            target_value = sustainHeight + onBumpHeight * (1 - ((envTime - onAttackSpeed) / onDecaySpeed));
        }
        else{
            // sustain region
            target_value = sustainHeight;
        }
    }
    else{
        if(envTime < offAttackSpeed){
            // release attack region
            target_value = sustainHeight + offBumpHeight * (envTime / offAttackSpeed);
        }
        else if(envTime < (offAttackSpeed + offDecaySpeed)){
            // release decay region
            target_value = (sustainHeight + offBumpHeight) * (1 - ((envTime - offAttackSpeed) / offDecaySpeed));
        }
        else{
            // closed region
            target_value = 0;
        }
    }

    samplesCtr++;

    // limit and slew
    target_value = target_value > 1.f ? 1.f : target_value < 0.f ? 0.f : target_value;
    outputs[0].value = slew*outputs[0].value + (1-slew)*target_value;
}
