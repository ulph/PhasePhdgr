#ifndef MODULE_HPP
#define MODULE_HPP

class Module
{
protected:
    uint32_t time;
    std::vector<float> inputs;
    float output;

public:
    virtual void process(uint32_t fs) = 0;
    uint32_t getTime() { return time; }
    void setTime(uint32_t t) { time = t; }
    float getOutput() { return output; }
    void setInput(int inputPad, float value) {
        if(inputPad >= 0 && inputPad < inputs.size()) {
            inputs[inputPad] = value;
        }
    }
    int getNumInputPads() { return (int)inputs.size(); }
};

class Phase : public Module
{
public:
    Phase() {
        inputs.push_back(440.0f);
    }
    void process(uint32_t fs)
    {
        // Get phase
        float f = inputs[0];
        uint32_t samplesPerPeriod = uint32_t((float)fs / f);
        float phase = (float(time % samplesPerPeriod) / float(samplesPerPeriod)) * 2.0f - 1.0f;
        output = phase;
    }
    constexpr static const char *desc =
        "PHASE - outpus current phase (-1 .. 1) (sawtooth)\n" \
        "  INPUTS:\n" \
        "    0 - Frequency (Hz)\n\n";
};

class Square : public Module
{
public:
    Square() {
        inputs.push_back(0.0f);
        inputs.push_back(1.0f);
    }
    void process(uint32_t fs)
    {
        // Get phase
        float phase = inputs[0];
        
        // Duty cycle (PWM)
        float duty = inputs[1];
        if(duty < 0.0f) duty = -duty;
        
        // Make Square from phase
        if(phase < -1.0f+duty) output = -1.0f;
        else if(phase < 0.0f) output =  0.0f;
        else if(phase < duty) output =  1.0f;
        else output =  0.0f;
    }
    constexpr static const char *desc =
        "SQUARE - forms square wave from incoming phase\n" \
        "  INPUTS:\n" \
        "    0 - Phase      (-1 ... 1)\n" \
        "    1 - Duty cycle (-1 ... 1)\n\n";
};

class Add : public Module
{
public:
    Add() {
        inputs.push_back(0.0f);
        inputs.push_back(0.0f);
    }
    void process(uint32_t fs) {
        output = inputs[0] + inputs[1];
    }
    constexpr static const char *desc =
        "ADD - adds two inputs\n" \
        "  INPUTS:\n" \
        "    0 - First term to add\n" \
        "    1 - Second term to add\n\n";
};


class Mul : public Module
{
public:
    Mul() {
        inputs.push_back(0.0f);
        inputs.push_back(0.0f);
    }
    void process(uint32_t fs) {
        output = inputs[0] * inputs[1];
    }
    constexpr static const char *desc =
        "MUL - multiplies two inputs\n" \
        "  INPUTS:\n" \
        "    0 - First factor to multiply\n" \
        "    1 - Second factor to multiply\n\n";
};

class Clamp : public Module
{
public:
    Clamp() { inputs.push_back(0.0f); }
    void process(uint32_t fs) {
        output = inputs[0];
        if(output < -1.0f) output = -1.0f;
        else if(output > 1.0f) output = 1.0f;
    }
    constexpr static const char *desc =
        "CLAMP - clamps input between -1 and 1\n" \
        "  INPUTS:\n" \
        "    0 - Input value\n\n";
};

class Quant8 : public Module
{
public:
    Quant8() { inputs.push_back(0.0f); }
    void process(uint32_t fs) {
        const float invStepSize = 128.0f;
        const float stepSize = 1.0f / invStepSize;
        
        float tmp = roundf(inputs[0] * invStepSize);
        if(tmp < -invStepSize) tmp = -invStepSize;
        else if(tmp > invStepSize-1) tmp = invStepSize-1;
        tmp = tmp * stepSize;
        
        output = tmp;
    }
    constexpr static const char *desc =
        "QUANT8 - 8-bit quantization\n" \
        "  INPUTS:\n" \
        "    0 - Input value\n\n";
};

#endif
