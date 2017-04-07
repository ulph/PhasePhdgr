#include "PhasePhckr.h"
#include <string.h>

using namespace PhasePhckr;

/* MPEVoice */

MPEVoice::MPEVoice() : slewFactor(0.995f) {
}

void MPEVoice::calculatePitchHz() {
    float bendSemi = st.glideX > 0 ? st.glideX * cfg.pitchRangeUp : st.glideX * cfg.pitchRangeDown;
    st.pitchHz = NoteToHz(rootNote + bendSemi);
}

void MPEVoice::on(int note, float velocity) {
    st.gate = true;
    st.samplesSinceGateOn = 0;
    st.samplesSinceGateOff = 0;
    rootNote = note;
    st.strikeZ = velocity;
    calculatePitchHz();
}

void MPEVoice::off(int note, float velocity) {
    st.gate = false;
    st.samplesSinceGateOff = 0; 
    st.liftZ = velocity;
}

void MPEVoice::glide(float glide) {
    tg.glideX = glide;
}

void MPEVoice::slide(float slide) {
    tg.slideY = slide;
}

void MPEVoice::press(float press) {
    tg.pressZ = press;
}

void MPEVoice::update() {
    st.samplesSinceGateOn++;
    st.samplesSinceGateOff++;
    st.glideX = slewFactor * st.glideX + (1-slewFactor) * tg.glideX;
    st.slideY = slewFactor * st.slideY + (1-slewFactor) * tg.slideY;
    st.pressZ = slewFactor * st.pressZ + (1-slewFactor) * tg.pressZ;
    calculatePitchHz();
}

void MPEVoice::reset(){
    memset(&st, 0, sizeof(st));
    memset(&tg, 0, sizeof(tg));
    memset(&cfg, 0, sizeof(cfg));
}

const MPEVoiceState & MPEVoice::getState() {
    return st;
}


/* ComponentI */

int ComponentI::numberOfInputPorts(){
    return inputPortNames.size();
}

int ComponentI::numberOfOutputPorts(){
    return outputPortNames.size();
}

int ComponentI::getInputPortNumber(char * name){
    return findPortNumber(inputPortNames, name);
}

int ComponentI::getOutputPortNumber(char * name){
    return findPortNumber(outputPortNames, name);
}

int ComponentI::findPortNumber(const std::vector<const char *> &portNames, const char * portName){
    int ret = -1;
    int i=0;
    for(auto &v : portNames){
        if(strcmp(v, portName)){
            return i;
        }
        i++;
    }
    return ret;
}
