#ifndef MODULE_HPP
#define MODULE_HPP

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
    float getOutput(int outputPad) { return outputs[outputPad].value; }
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
        uint32_t samplesPerPeriod = uint32_t((float)fs / f);
        float phase = (float(time % samplesPerPeriod) / float(samplesPerPeriod)) * 2.0f - 1.0f;
        outputs[0].value = phase;
    }
};

class Square : public Module
{
public:
    Square() {
        inputs.push_back(Pad("phase"));
        inputs.push_back(Pad("dutycycle", 1.0f));
        outputs.push_back(Pad("square", 0.0f));
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

#endif
