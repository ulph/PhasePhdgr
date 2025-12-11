#include "rlc.hpp"
#include "inlines.hpp"

RcLp::RcLp()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("wc", 16000.f, "hz"));
    outputs.push_back(Pad("out"));
}

void RcLp::processSample(int sample)
{
    float x1 = inputs[0].values[sample];
    float y0 = outputs[0].values[sample];
    float wc = inputs[1].values[sample];
    float y1 = CalcRcLp(x1, y0, wc, fsInv);
    outputs[0].values[sample] = y1;
}


RcHp::RcHp() 
    : x(0.0f)
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("wc", 40.f, "hz"));
    outputs.push_back(Pad("out"));
}

void RcHp::processSample(int sample)
{
    float x1 = inputs[0].values[sample];
    float x0 = x;
    float y0 = outputs[0].values[sample];
    float wc = inputs[1].values[sample];
    float y1 = CalcRcHp(x1, x0, y0, wc, fsInv);
    outputs[0].values[sample] = y1;
    x = x1;
}

Lag::Lag()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("wc_up", "hz"));
    inputs.push_back(Pad("wc_down", "hz"));
    outputs.push_back(Pad("out"));
}

void Lag::processSample(int sample)
{
    float x1 = inputs[0].values[sample];
    float y0 = outputs[0].values[sample];
    float wc = limitLow(inputs[1].values[sample]);
    if (x1 < y0) wc = limitLow(inputs[2].values[sample]);
    float y1 = CalcRcLp(x1, y0, wc, fsInv);
    outputs[0].values[sample] = y1;
}


LeakyIntegrator::LeakyIntegrator()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("freq", "hz"));
    inputs.push_back(Pad("dcRemoval", 0.125f));
    outputs.push_back(Pad("out"));
}

void LeakyIntegrator::processSample(int sample)
{
    float value = inputs[0].values[sample];
    float freq = limit(inputs[1].values[sample], 1.f, fs*0.5f);
    float nFreq = 2.f*freq * fsInv;
    float prop_leak = nFreq * 0.01f;
    float leak = 1.f - prop_leak;
    float dcBlockWc = freq*limitLow(inputs[2].values[sample], 0.0078125f);

    last_cumSum = cumSum;
    last_output = outputs[0].values[sample];

    cumSum = cumSum*leak + value;
    outputs[0].values[sample] = CalcRcHp(cumSum, last_cumSum, last_output, dcBlockWc, fsInv);;
}


RateLimiter::RateLimiter()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("wc_up", "hz")); // TODO, not true frequencies
    inputs.push_back(Pad("wc_down", "hz"));
    outputs.push_back(Pad("out"));
}

void RateLimiter::processSample(int sample)
{
    float x = inputs[0].values[sample];
    float y = outputs[0].values[sample];

    float rateLimitUp = limitLow(inputs[1].values[sample]) * fsInv;
    float rateLimitDown = limitLow(inputs[2].values[sample]) * fsInv;

    if (x > y && (x - y) > rateLimitUp) y += rateLimitUp;
    else if( x < y && (y - x) > rateLimitDown) y -= rateLimitDown;
    else y = x;

    outputs[0].values[sample] = y;
}
