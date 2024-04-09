#include "conversion.hpp"
#include <math.h>

TempoToTime::TempoToTime() {
    inputs.push_back(Pad("beats"));
    inputs.push_back(Pad("bpm", "bpm"));
    // TODO, a reset thingy
    outputs.push_back(Pad("period", "seconds"));
    outputs.push_back(Pad("freq", "hz"));
}

void TempoToTime::processSample(int sample) {
    float beats = inputs[0].values[sample];
    float bpm = inputs[1].values[sample];
    if(beats <= 0 || bpm <= 0) {
        outputs[0].values[sample] = 0;
        outputs[1].values[sample] = (float)fs;
        return;
    }
    float period = 60.f * beats/bpm;
    outputs[0].values[sample] = period;
    outputs[1].values[sample] = 1.f/period;
}


Transpose::Transpose() {
    inputs.push_back(Pad("freq", "hz"));
    inputs.push_back(Pad("octave", 0.f));
    inputs.push_back(Pad("semi", 0.f));
    inputs.push_back(Pad("cent", 0.f));
    outputs.push_back(Pad("freq", "hz"));
}

void Transpose::processSample(int sample) {
    float freq_in = inputs[0].values[sample];
    if(freq_in <= 0){
        outputs[0].values[sample] = 0.f;
        return;
    }

    float octave = inputs[1].values[sample];
    float semi = inputs[2].values[sample];
    float cent = inputs[3].values[sample];

    const float cent_to_semi = 1.f / 100.f;
    const float semi_to_oct = 1.f / 12.f;

    semi += cent * cent_to_semi;
    octave += semi * semi_to_oct;

    float freq_out = freq_in * powf(2.0, octave);
    outputs[0].values[sample] = freq_out;
}
