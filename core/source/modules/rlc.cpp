#include "rlc.hpp"
#include "inlines.hpp"

RcLp::RcLp()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("wc", 16000.f, "hz"));
    outputs.push_back(Pad("out"));
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
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("wc", 40.f, "hz"));
    outputs.push_back(Pad("out"));
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

Lag::Lag()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("wc_up", "hz"));
    inputs.push_back(Pad("wc_down", "hz"));
    outputs.push_back(Pad("out"));
}

void Lag::process()
{
    float x1 = inputs[0].value;
    float y0 = outputs[0].value;
    float wc = limitLow(inputs[1].value);
    if (x1 < y0) wc = limitLow(inputs[2].value);
    float y1 = CalcRcLp(x1, y0, wc, fsInv);
    outputs[0].value = y1;
}


LeakyIntegrator::LeakyIntegrator()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("freq", "hz"));
    inputs.push_back(Pad("dcRemoval", 0.125f));
    outputs.push_back(Pad("out"));
}

void LeakyIntegrator::process()
{
    float value = inputs[0].value;
    float freq = limit(inputs[1].value, 1.f, fs*0.5f);
    float nFreq = 2.f*freq * fsInv;
    float prop_leak = nFreq * 0.01f;
    float leak = 1.f - prop_leak;
    float dcBlockWc = freq*limitLow(inputs[2].value, 0.0078125f);

    last_cumSum = cumSum;
    last_output = outputs[0].value;

    cumSum = cumSum*leak + value;
    outputs[0].value = CalcRcHp(cumSum, last_cumSum, last_output, dcBlockWc, fsInv);;
}


RateLimiter::RateLimiter()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("wc_up", "hz")); // TODO, not true frequencies
    inputs.push_back(Pad("wc_down", "hz"));
    outputs.push_back(Pad("out"));
}

void RateLimiter::process()
{
    float x = inputs[0].value;
    float y = outputs[0].value;

    float rateLimitUp = limitLow(inputs[1].value) * fsInv;
    float rateLimitDown = limitLow(inputs[2].value) * fsInv;

    if (x > y && (x - y) > rateLimitUp) y += rateLimitUp;
    else if( x < y && (y - x) > rateLimitDown) y -= rateLimitDown;
    else y = x;

    outputs[0].value = y;
}