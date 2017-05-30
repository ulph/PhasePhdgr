#ifndef TEMPO_HPP
#define TEMPO_HPP

class TempoToTime : public Module
{
public:
    TempoToTime() {
        inputs.push_back(Pad("beats"));
        inputs.push_back(Pad("bpm"));
        // TODO, a reset thingy
        outputs.push_back(Pad("period"));
        outputs.push_back(Pad("freq"));
    }
    void process(uint32_t fs) {
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
    static Module* factory() { return new TempoToTime(); }
};

#endif
