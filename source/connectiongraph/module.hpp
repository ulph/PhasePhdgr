#ifndef MODULE_HPP
#define MODULE_HPP

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <vector>

struct Pad
{
    float value;
    std::string name;
    Pad(const char *name) : name(name), value(0.0f) {}
    Pad(const char *name, float value) : name(name), value(value) {}
};

class Module
{
protected:
    uint32_t time;
    std::vector<Pad> inputs;
    std::vector<Pad> outputs;

public:
    virtual void process(uint32_t fs) = 0;
    uint32_t getTime() { return time; }
    void setTime(uint32_t t) { time = t; }
    float getOutput(int outputPad) { 
        if(outputPad >= 0 && outputPad < outputs.size()){
            return outputs[outputPad].value; 
        }
        return 0;
        }
    void setInput(int inputPad, float value) {
        if(inputPad >= 0 && inputPad < inputs.size()) {
            inputs[inputPad].value = value;
        }
    }
    int getNumInputPads() { return (int)inputs.size(); }
    int getNumOutputPads() { return (int)outputs.size(); }
};

class Phase : public Module
{
public:
    Phase() {
        inputs.push_back(Pad("freq", 440.0f));
        outputs.push_back(Pad("phase"));
    }
    void process(uint32_t fs)
    {
        // Get phase
        float f = inputs[0].value;
        float p = outputs[0].value;
        if(fs){
            p += 2 * f / (float)fs;
        }
        while(p > 1){p-=2;}
        while(p < -1){p+=2;}
        outputs[0].value = p;        
    }
};

class Sine : public Module
{
public:
    Sine() {
        inputs.push_back(Pad("phase", 0.0f));
        outputs.push_back(Pad("sine"));
    }
    void process(uint32_t fs)
    {
        outputs[0].value = (float)sin(M_PI * inputs[0].value);
    }
};

class AtanSaturator : public Module
{
public:
    AtanSaturator() {
        inputs.push_back(Pad("in", 0.0f));
        inputs.push_back(Pad("prescaler", 1.0f));
        outputs.push_back(Pad("saturated"));
    }
    void process(uint32_t fs)
    {
        float scale = (float)fmax(inputs[1].value, 0.01);
        outputs[0].value = atanf(inputs[0].value * scale) / atanf(scale);
    }
};

class Constant : public Module
{
public:
    Constant() {
        inputs.push_back(Pad("constant", 1.0f));
        outputs.push_back(Pad("constant"));
    }
    void process(uint32_t fs)
    {
        outputs[0].value = outputs[0].value;
    }
};

class Square : public Module
{
public:
    Square() {
        inputs.push_back(Pad("phase", 0.0f));
        inputs.push_back(Pad("dutycycle", 1.0f));
        outputs.push_back(Pad("square"));
    }
    void process(uint32_t fs)
    {
        // Get phase
        float phase = inputs[0].value;
        
        // Duty cycle (PWM)
        float duty = inputs[1].value;
        if(duty < 0.0f) duty = -duty;
        
        // Make Square from phase
        if(phase < -1.0f+duty) outputs[0].value = -1.0f;
        else if(phase < 0.0f) outputs[0].value =  0.0f;
        else if(phase < duty) outputs[0].value =  1.0f;
        else outputs[0].value =  0.0f;
    }
};

class Add : public Module
{
public:
    Add() {
        inputs.push_back(Pad("in1"));
        inputs.push_back(Pad("in2"));
        outputs.push_back(Pad("sum"));
    }
    void process(uint32_t fs) {
        outputs[0].value = inputs[0].value + inputs[1].value;
    }
};

class Mul : public Module
{
public:
    Mul() {
        inputs.push_back(Pad("in1"));
        inputs.push_back(Pad("in2"));
        outputs.push_back(Pad("prod"));
    }
    void process(uint32_t fs) {
        outputs[0].value = inputs[0].value * inputs[1].value;
    }
};

class Clamp : public Module
{
public:
    Clamp() {
        inputs.push_back(Pad("in"));
        outputs.push_back(Pad("clamp"));
    }
    void process(uint32_t fs) {
        outputs[0] = inputs[0];
        if(outputs[0].value < -1.0f) outputs[0].value = -1.0f;
        else if(outputs[0].value > 1.0f) outputs[0].value = 1.0f;
    }
};

class Lag : public Module
{
public:
    Lag() {
        inputs.push_back(Pad("input"));
        inputs.push_back(Pad("amount"));
        outputs.push_back(Pad("output"));
    }
    void process(uint32_t fs) {
        outputs[0].value = outputs[0].value * inputs[1].value + inputs[0].value * ( 1 - inputs[1].value );
    }
};

class Rectifier : public Module
{
public:
    Rectifier() {
        inputs.push_back(Pad("input"));
        outputs.push_back(Pad("output"));
    }
    void process(uint32_t fs) {
        float v = inputs[0].value;
        outputs[0].value = v >= 0 ? v : -v;
    }
};

class CrossFade : public Module
{
public:
    CrossFade() {
        inputs.push_back(Pad("first"));
        inputs.push_back(Pad("second"));
        inputs.push_back(Pad("crossfade"));
        outputs.push_back(Pad("output"));
    }
    void process(uint32_t fs) {
        outputs[0].value = inputs[2].value*inputs[0].value + (1-inputs[2].value)*inputs[1].value;
    }
};

class FoldBack : public Module
{
public:
    FoldBack() {
        inputs.push_back(Pad("input"));
        inputs.push_back(Pad("threshhold", 1.f));
        outputs.push_back(Pad("output"));
    }
    void process(uint32_t fs) {
        float v = inputs[0].value;
        float t = fabs(inputs[1].value);
        float d = fabs(inputs[0].value) - inputs[1].value;
        if(d > 0){
            if(v >= 0){
                outputs[0].value = v-2*d;
            }
            else{
                outputs[0].value = v+2*d;
            }
        }
        else{
            outputs[0].value = inputs[0].value;
        }
    }
};

class Quant8 : public Module
{
public:
    Quant8() {
        inputs.push_back(Pad("in"));
        outputs.push_back(Pad("quant"));
    }
    void process(uint32_t fs) {
        const float invStepSize = 128.0f;
        const float stepSize = 1.0f / invStepSize;
        
        float tmp = roundf(inputs[0].value * invStepSize);
        if(tmp < -invStepSize) tmp = -invStepSize;
        else if(tmp > invStepSize-1) tmp = invStepSize-1;
        tmp = tmp * stepSize;
        
        outputs[0].value = tmp;
    }
};

class Noise : public Module
{
private:
    uint32_t val;
public:
    Noise() {
        outputs.push_back(Pad("out"));
        val = 01135; // seed
    }

    void process(uint32_t fs) {
        outputs[0].value = ((float)((val & 0x7fffffff) - 0x40000000)) *
                          (float)(1.0 / 0x40000000);
        val = val * 435898247 + 382842987;
    }
};

class CamelEnvelope : public Module
{
    public:
    CamelEnvelope():
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
    
    void process(uint32_t fs) {
        float new_gate = inputs[0].value;
        float onBumpHeight = inputs[1].value;
        float onAttackSpeed = inputs[2].value;
        float onDecaySpeed = inputs[3].value;
        float sustainHeight = inputs[4].value;
        float offBumpHeight = inputs[5].value;
        float offAttackSpeed = inputs[6].value;
        float offDecaySpeed = inputs[7].value;

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

    private:
        float gate;
        float target_value;
        float slew;
        int samplesCtr;
};

#endif
