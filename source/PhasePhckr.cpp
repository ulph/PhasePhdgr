#include "PhasePhckr.h"
#include "PhasePhckr_internal.h"
#include <string.h>
#include "SynthVoices/ExSynthVoice.h"

namespace PhasePhckr{

/* MPEVoice */

MPEVoice::MPEVoice() : slewFactor(0.995f), rootNote(0) {
    reset();
}

void MPEVoice::calculatePitchHz() {
    float bendSemi = st.glideX > 0 ? st.glideX * cfg.pitchRangeUp : st.glideX * cfg.pitchRangeDown;
    st.pitchHz = NoteToHz(rootNote + bendSemi);
}

void MPEVoice::on(int note, float velocity) {
    st.gate = 1.f;
    rootNote = note;
    st.strikeZ = velocity;
    calculatePitchHz();
}

void MPEVoice::off(int note, float velocity) {
    st.gate = 0.f;
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
    st.glideX = slewFactor * st.glideX + (1.0f-slewFactor) * tg.glideX;
    st.slideY = slewFactor * st.slideY + (1.0f-slewFactor) * tg.slideY;
    st.pressZ = slewFactor * st.pressZ + (1.0f-slewFactor) * tg.pressZ;
    calculatePitchHz();
}

void MPEVoice::reset(){
    memset(&st, 0, sizeof(st));
    memset(&tg, 0, sizeof(tg));
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

void VoiceBus::handleNoteOnOff(int channel, int note, float velocity, bool on)
{
    SynthVoiceI *v = (*voices)[channel];
    on ? v->mpe.on(note, velocity) : v->mpe.off(note, velocity);
}

void VoiceBus::handleX(int channel, float position){
    (*voices)[channel]->mpe.glide(position);
}

void VoiceBus::handleY(int channel, float position){
    (*voices)[channel]->mpe.slide(position);
}

void VoiceBus::handleZ(int channel, float position){
    (*voices)[channel]->mpe.press(position);
}


Synth::Synth() {
    for(int i=0; i<16; ++i){
        SynthVoiceI* v = new ExConnectionGraphVoice();
        v->reset();
        voices.push_back(v);
    }
    voiceBus = VoiceBus(&voices);
}

void Synth::update(float * buffer, int numSamples, float sampleRate)
{
    for (auto & v : voices) v->update(buffer, numSamples, sampleRate);
    for (auto & e : effects) e->update(buffer, numSamples, sampleRate);
}

}