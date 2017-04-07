#include "CamelEnvelope.h"

using namespace PhasePhckr;
using namespace Components;

CamelEnvelope::CamelEnvelope():
    onBumpHeight(1.f),
    offBumpHeight(1.f),
    onAttackSpeed(0.01f),
    onDecaySpeed(0.05f),
    offAttackSpeed(0.05f),
    offDecaySpeed(0.05f),
    sustainHeight(0.f),
    samplesCtr(0),
    value(0.f),
    target_value(0.f),
    slew(0.9f)
{
    outputPortNames.push_back("envelopeValue");

    inputPortNames.push_back("gate");
    inputPortNames.push_back("onBumpHeight");
    inputPortNames.push_back("onAttackSpeed");
    inputPortNames.push_back("onDecaySpeed");
    inputPortNames.push_back("sustainHeight");
    inputPortNames.push_back("offBumpHeight");
    inputPortNames.push_back("offAttackSpeed");
    inputPortNames.push_back("offDecaySpeed");
}

void CamelEnvelope::put(int port, float value){
    float * target = nullptr;
    switch(port){
        case 0: 
        target = &gate; 
        if( (gate >= 1.0 && value < 1.0) || (gate < 1.0 && value >= 1.0) ){
            samplesCtr = 0;
        } 
        break;
        case 1: target = &onBumpHeight; break;
        case 2: target = &onAttackSpeed; break;
        case 3: target = &onDecaySpeed; break;
        case 4: target = &sustainHeight; break;
        case 5: target = &offBumpHeight; break;
        case 6: target = &offAttackSpeed; break;
        case 7: target = &offDecaySpeed; break;
    }
    if(target) *target = value;
}

float CamelEnvelope::get(int port){
    if(port == 0)
        return value;
    return 0;
}

void CamelEnvelope::update(){
    // initial state handling
    if(gate == -1){
        value = 0.f;
        return;
    }

    samplesCtr++;
    float time = (float)samplesCtr / sampleRate;
    if(gate){
        if(time < onAttackSpeed){
            // attack region
            target_value = (sustainHeight + onBumpHeight) * (time / onAttackSpeed);
        }
        else if(time < (onAttackSpeed + onDecaySpeed)){
            // decay region
            target_value = sustainHeight + onBumpHeight * (1 - ((time - onAttackSpeed) / onDecaySpeed));
        }
        else{
            // sustain region
            target_value = sustainHeight;
        }
    }
    else{
        if(time < offAttackSpeed){
            // release attack region
            target_value = sustainHeight + offBumpHeight * (time / offAttackSpeed);
        }
        else if(time < (offAttackSpeed + offDecaySpeed)){
            // release decay region
            target_value = (sustainHeight + offBumpHeight) * (1 - ((time - offAttackSpeed) / offDecaySpeed));
        }
        else{
            // closed region
            target_value = 0;
        }
    }

    // limit
    target_value = target_value > 1.f ? 1.f : target_value < 0.f ? 0.f : target_value;

    // slew to make smooth - TODO blockSize and sampleRate invariant
    value = slew*value + (1-slew)*target_value;
}

void CamelEnvelope::reset(){
    // do naught for now
}

void CamelEnvelope::setSustainHeight(float level){
    sustainHeight = level;
}

void CamelEnvelope::setOnBumpHeight(float level){
    onBumpHeight = level;
}

void CamelEnvelope::setOffBumpHeight(float level){
    offBumpHeight = level;
}

void CamelEnvelope::setOnAttackSpeed(float speed){
    onAttackSpeed = speed;
}

void CamelEnvelope::setOffAttackSpeed(float speed){
    offAttackSpeed = speed;
}

void CamelEnvelope::setOnDecaySpeed(float speed){
    onDecaySpeed = speed;
}

void CamelEnvelope::setOffDecaySpeed(float speed){
    offDecaySpeed = speed;
}
