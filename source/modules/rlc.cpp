#include "rlc.hpp"

static float CalcRcLp(float x1, float y0, float wc, float fs) {
    float d = 2.0f * M_PI*wc / (float)fs;
    float a = d / (d + 1.0f);
    return a*y0 + (1.0f - a)*x1;
}

static float CalcRcHp(float x1, float x0, float y0, float wc, float fs) {
    float a = 1.0f / ((2.0f * M_PI*wc / (float)fs) + 1.0f);
    return a*y0 + a*(x1 - x0);
}

RcLp::RcLp()
{
    inputs.push_back(Pad("x1"));
    inputs.push_back(Pad("wc", 16000.f));
    outputs.push_back(Pad("y1"));
}

void RcLp::process(uint32_t fs)
{
    float x1 = inputs[0].value;
    float y0 = outputs[0].value;
    float wc = inputs[1].value;
    float y1 = CalcRcLp(x1, y0, wc, (float)fs);
    outputs[0].value = y1;
}

RcHp::RcHp() 
    : x(0.0f)
{
    inputs.push_back(Pad("x1"));
    inputs.push_back(Pad("wc", 40.f));
    outputs.push_back(Pad("y1"));
}

void RcHp::process(uint32_t fs)
{
    float x1 = inputs[0].value;
    float x0 = x;
    float y0 = outputs[0].value;
    float wc = inputs[1].value;
    float y1 = CalcRcHp(x1, x0, y0, wc, (float)fs);
    outputs[0].value = y1;
    x = x1;
}

OpenRcLp::OpenRcLp()
{
    inputs.push_back(Pad("x1")); // input
    inputs.push_back(Pad("y0"));
    inputs.push_back(Pad("wc", 16000.f));
    outputs.push_back(Pad("y1")); // output
}

void OpenRcLp::process(uint32_t fs)
{
    float x1 = inputs[0].value;
    float y0 = inputs[1].value;
    float wc = inputs[2].value;
    float y1 = CalcRcLp(x1, y0, wc, (float)fs);
    outputs[0].value = y1;
    outputs[0].value = x1;
}

OpenRcHp::OpenRcHp()
{
    inputs.push_back(Pad("x1")); // input
    inputs.push_back(Pad("x0"));
    inputs.push_back(Pad("y0"));
    inputs.push_back(Pad("wc", 40.f));
    outputs.push_back(Pad("y1")); // output
    outputs.push_back(Pad("x1"));
}

void OpenRcHp::process(uint32_t fs)
{
    float x1 = inputs[0].value;
    float x0 = inputs[1].value;
    float y0 = inputs[2].value;
    float wc = inputs[3].value;
    float y1 = CalcRcHp(x1, x0, y0, wc, (float)fs);
    outputs[0].value = y1;
    outputs[0].value = x1;
}
