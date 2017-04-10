#include "foldback.hpp"

FoldBack::FoldBack()
{
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("amount", 1.0f));
    inputs.push_back(Pad("prescalar", 1.0f));
    outputs.push_back(Pad("output"));
}

bool FoldBack::iterate(float *v, float scale) {
    float d = fabs(*v) - 1;
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

void FoldBack::process(uint32_t fs) {
    float v = inputs[2].value*inputs[0].value;
    float s = fmax(0.0f, fmin(1.0f, fabs(inputs[1].value)));
    for (int i = 0; i < 10; ++i) {
        if(iterate(&v, s)) break;
    }
    outputs[0].value = v;
}
