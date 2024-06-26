#include "foldback.hpp"
#include "inlines.hpp"
#include <math.h>

FoldBack::FoldBack()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("amount", 1.0f));
    inputs.push_back(Pad("prescalar", 1.0f));
    inputs.push_back(Pad("threshhold", 1.0f));
    outputs.push_back(Pad("out"));
}

bool FoldBack::iterate(float *v, float scale, float th) {
    float d = fabsf(*v) - th;
    if (d > 0) {
        if (*v >= 0) {
            *v = *v - (d + d*scale);
        }
        else {
            *v = *v + (d + d*scale);
        }
        return false;
    }
    else {
        return true;
    }
}

void FoldBack::process() {
    float v = inputs[2].value*inputs[0].value;
    float s = limit(inputs[1].value, 0.0f, 1.0f);
    float t = limit(inputs[3].value, 0.01f, 1.0f);
    for (int i = 0; i < 100; ++i) {
        if(iterate(&v, s, t)) break;
    }
    outputs[0].value = v;
}


Wrap::Wrap()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("prescalar", 1.0f));
    inputs.push_back(Pad("threshhold", 1.0f));
    outputs.push_back(Pad("out"));
}

bool Wrap::iterate(float *v, float th) {
    float d = fabsf(*v) - th;
    if (d > 0) {
        if (*v >= 0) {
            *v = -th + d;
        }
        else {
            *v = th - d;
        }
        return false;
    }
    else {
        return true;
    }
}

void Wrap::process() {
    float s = limitLow(inputs[1].value, 0.01f);
    float t = limitLow(inputs[2].value, 0.01f);
    float v = s*inputs[0].value;
    for (int i = 0; i < 100; ++i) {
        if (iterate(&v, t)) break;
    }
    outputs[0].value = v;
}
