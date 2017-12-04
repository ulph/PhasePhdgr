#include "clampinv.hpp"

ClampInv::ClampInv() {
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("low", 0.0f));
    inputs.push_back(Pad("high", 1.0f));
    outputs.push_back(Pad("output"));
}

void ClampInv::process() {
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
