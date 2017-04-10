#include "foldback.hpp"

FoldBack::FoldBack()
{
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("amount", 0.5f));
    inputs.push_back(Pad("prescalar", 1.0f));
    outputs.push_back(Pad("output"));
}

bool FoldBack::iterate(float *v) {
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

void FoldBack::process(uint32_t fs) {
    float v = inputs[2].value*inputs[0].value;
    for (int i = 0; i < 20; ++i) {
        if(iterate(&v)) break;
    }
    outputs[0].value = v;
}
