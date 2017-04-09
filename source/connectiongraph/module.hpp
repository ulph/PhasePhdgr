#ifndef MODULE_HPP
#define MODULE_HPP

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <vector>

struct Pad
{
    float value;
    float floatingValue;
    bool isFloating;
    std::string name;
    Pad(const char *name) : name(name), value(0.0f), floatingValue(0.0f), isFloating(true){}
    Pad(const char *name, float floatingValue) : name(name), value(0.0f), floatingValue(floatingValue), isFloating(true) {}
    void setFloatingValue(float value) {
        floatingValue = value;
    }
};

class Module
{
protected:
    uint32_t time;
    std::vector<Pad> inputs;
    std::vector<Pad> outputs;

public:
    virtual void doProcess(uint32_t fs) {
        process(fs);
        postProcess();
    }
    virtual void process(uint32_t fs) = 0;
    virtual void postProcess() {
        for (auto & i : inputs) {
            i.value = i.isFloating ? i.floatingValue : 0;
            i.isFloating = true;
        }
    };
    uint32_t getTime() { return time; }
    void setTime(uint32_t t) { time = t; }
    float getOutput(int outputPad) { 
        if(outputPad >= 0 && outputPad < outputs.size()){
            return outputs[outputPad].value;
        }
        return 0;
    }
    virtual void setInput(int inputPad, float value) {
        if(inputPad >= 0 && inputPad < inputs.size()) {
            inputs[inputPad].value += value;
            inputs[inputPad].isFloating = false;
        }
    }
    int getNumInputPads() { return (int)inputs.size(); }
    int getNumOutputPads() { return (int)outputs.size(); }
    void setFloatingValue(int inputPad, float value) {
        if (inputPad >= 0 && inputPad < inputs.size()) {
            inputs[inputPad].floatingValue = value;
        }
    }
};

class Phase : public Module
{
public:
    Phase() {
        inputs.push_back(Pad("freq"));
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
        inputs.push_back(Pad("phase"));
        outputs.push_back(Pad("sine"));
    }
    void process(uint32_t fs)
    {
        outputs[0].value = (float)sin(M_PI * inputs[0].value);
    }
};

class SaturatorAtan : public Module
{
public:
    SaturatorAtan() {
        inputs.push_back(Pad("in"));
        inputs.push_back(Pad("prescaler", 1.0f));
        outputs.push_back(Pad("saturated"));
    }
    void process(uint32_t fs)
    {
        float scale = (float)fmax(inputs[1].value, 0.01);
        outputs[0].value = atanf(inputs[0].value * scale) / atanf(scale);
    }
};

class Square : public Module
{
public:
    Square() {
        inputs.push_back(Pad("phase"));
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

class SymPow : public Module
{
public:
    SymPow() {
        inputs.push_back(Pad("base", 1.0));
        inputs.push_back(Pad("exp", 1.0));
        outputs.push_back(Pad("pow"));
    }
    void process(uint32_t fs) {
        float v = abs(inputs[0].value);
        float sign = inputs[0].value >= 0 ? 1 : -1;
        outputs[0].value = sign*powf(v, inputs[1].value);
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
        inputs.push_back(Pad("amount", 0.9));
        outputs.push_back(Pad("output"));
    }
    void process(uint32_t fs) {
        outputs[0].value = outputs[0].value * inputs[1].value + inputs[0].value * ( 1 - inputs[1].value );
    }
};

class Abs : public Module
{
public:
    Abs() {
        inputs.push_back(Pad("input"));
        outputs.push_back(Pad("abs"));
    }
    void process(uint32_t fs) {
        outputs[0].value = fabs(inputs[0].value);
    }
};

class ScaleShift : public Module
{
public:
    ScaleShift() {
        inputs.push_back(Pad("input"));
        inputs.push_back(Pad("scale", 2.f));
        inputs.push_back(Pad("shift", -1.f));
        outputs.push_back(Pad("abs"));
    }
    void process(uint32_t fs) {
        outputs[0].value = inputs[0].value * inputs[1].value + inputs[2].value;
    }
};

class ClampInv : public Module
{
public:
    ClampInv() {
        inputs.push_back(Pad("in"));
        inputs.push_back(Pad("low", 0.0f));
        inputs.push_back(Pad("high", 1.0f));
        outputs.push_back(Pad("out"));
    }
    void process(uint32_t fs) { 
        float v = inputs[0].value;
        float lo = inputs[1].value;
        float hi = inputs[2].value;
        // clamp if outside bound
        if (v < lo) {
            outputs[0].value = lo;
        }
        else if (v > hi) {
            outputs[0].value = hi;
        }
        // invert inside of bound
        else if (hi >= lo) {
            outputs[0].value = hi - (v-lo);
        }
        // handle nonsensical settings
        else {
            outputs[0].value = 0;
        }
    }
};

class CrossFade : public Module
{
public:
    CrossFade() {
        inputs.push_back(Pad("first"));
        inputs.push_back(Pad("second"));
        inputs.push_back(Pad("crossfade", 0.5));
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
        inputs.push_back(Pad("amount", 0.5f));
        inputs.push_back(Pad("prescalar", 1.0f));
        outputs.push_back(Pad("output"));
    }
    bool iterate(float *v) {
        float d = fabs(*v) - 1;
        float s = fmax(0.1, fmin(1, fabs(inputs[1].value)));
        if (d > 0) {
            if (*v >= 0) {
                *v = *v - (d + d*s);
            }
            else {
                *v = *v + (d + d*s);
            }
            return false;
        }
        else {
            return true;
        }
    }
    void process(uint32_t fs) {
        float v = inputs[2].value*inputs[0].value;
        for (int i = 0; i < 20; ++i) {
            if(iterate(&v)) break;
        }
        outputs[0].value = v;
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

    private:
        float gate;
        float target_value;
        float slew;
        int samplesCtr;
};

#endif
