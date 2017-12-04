#include "crossfade.hpp"

CrossFade::CrossFade() {
    inputs.push_back(Pad("first"));
    inputs.push_back(Pad("second"));
    inputs.push_back(Pad("crossfade", 0.5));
    outputs.push_back(Pad("output"));
}
void CrossFade::process() {
    outputs[0].value = inputs[2].value*inputs[0].value + (1-inputs[2].value)*inputs[1].value;
}
