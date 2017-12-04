#include "rlc.hpp"

RcLp::RcLp()
{
    inputs.push_back(Pad("x1"));
    inputs.push_back(Pad("wc", 16000.f));
    outputs.push_back(Pad("y1"));
}

void RcLp::process()
{
    float x1 = inputs[0].value;
    float y0 = outputs[0].value;
    float wc = inputs[1].value;
    float y1 = CalcRcLp(x1, y0, wc, fsInv);
    outputs[0].value = y1;
}

RcHp::RcHp() 
    : x(0.0f)
{
    inputs.push_back(Pad("x1"));
    inputs.push_back(Pad("wc", 40.f));
    outputs.push_back(Pad("y1"));
}

void RcHp::process()
{
    float x1 = inputs[0].value;
    float x0 = x;
    float y0 = outputs[0].value;
    float wc = inputs[1].value;
    float y1 = CalcRcHp(x1, x0, y0, wc, fsInv);
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

void OpenRcLp::process()
{
    float x1 = inputs[0].value;
    float y0 = inputs[1].value;
    float wc = inputs[2].value;
    float y1 = CalcRcLp(x1, y0, wc, fsInv);
    outputs[0].value = y1;
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

void OpenRcHp::process()
{
    float x1 = inputs[0].value;
    float x0 = inputs[1].value;
    float y0 = inputs[2].value;
    float wc = inputs[3].value;
    float y1 = CalcRcHp(x1, x0, y0, wc, fsInv);
    outputs[0].value = y1;
    outputs[1].value = x1;
}

Lag::Lag()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("wc_up"));
    inputs.push_back(Pad("wc_down"));
    outputs.push_back(Pad("out"));
}

void Lag::process()
{
    float x1 = inputs[0].value;
    float y0 = outputs[0].value;
    float wc = inputs[1].value;
    if (x1 < y0) wc = inputs[2].value;
    float y1 = CalcRcLp(x1, y0, wc, fsInv);
    outputs[0].value = y1;
}
