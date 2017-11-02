#include "conversion.hpp"

TempoToTime::TempoToTime() {
    inputs.push_back(Pad("beats"));
    inputs.push_back(Pad("bpm"));
    // TODO, a reset thingy
    outputs.push_back(Pad("period"));
    outputs.push_back(Pad("freq"));
}

void TempoToTime::process(uint32_t fs) {
    float beats = inputs[0].value;
    float bpm = inputs[1].value;
    if(beats <= 0 || bpm <= 0) {
        outputs[0].value = 0;
        outputs[1].value = fs;
        return;
    }
    float period = 60.f * beats/bpm;
    outputs[0].value = period;
    outputs[1].value = 1.f/period;
}


Transpose::Transpose() {
    inputs.push_back(Pad("freq"));
    inputs.push_back(Pad("octave", 0.f));
    inputs.push_back(Pad("semi", 0.f));
    inputs.push_back(Pad("cent", 0.f));
    outputs.push_back(Pad("freq"));
}

void Transpose::process(uint32_t fs) {
    float freq_in = inputs[0].value;
    if(freq_in <= 0){
        outputs[0].value = 0;
        return;
    }

    float octave = inputs[1].value;
    float semi = inputs[2].value;
    float cent = inputs[3].value;

    semi += cent / 100.f;
    octave += semi / 12.f;

    float freq_out = freq_in * powf(2.0, octave);
    outputs[0].value = freq_out;
}
