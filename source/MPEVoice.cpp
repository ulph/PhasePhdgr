#include "MPEVoice.hpp"

#include <math.h>
#include <string.h>

namespace PhasePhckr {

static float NoteToHz(float note) {
    if (note > 127.f) {
        return 0;
    }
    float hz = 440.f * powf(2.f, (note - 69.f) / 12.f);
    return hz;
}

MPEVoice::MPEVoice() : slewFactor(0.995f), rootNote(0), age(0){
    reset();
}

void MPEVoice::calculatePitchHz() {
    float bendSemi = st.glideX > 0 ? st.glideX * cfg.pitchRangeUp : st.glideX * cfg.pitchRangeDown;
    st.pitchHz = NoteToHz(rootNote + bendSemi);
}

void MPEVoice::on(int note, float velocity) {
    age = 0;
    st.gate = 1.f;
    rootNote = note;
    st.strikeZ = velocity;
    calculatePitchHz();
}

void MPEVoice::off(int note, float velocity) {
    if (note == rootNote && st.gate) {
        age = 0;
        st.gate = 0.f;
        st.liftZ = velocity;
    }
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
    st.glideX = slewFactor * st.glideX + (1.0f - slewFactor) * tg.glideX;
    st.slideY = slewFactor * st.slideY + (1.0f - slewFactor) * tg.slideY;
    st.pressZ = slewFactor * st.pressZ + (1.0f - slewFactor) * tg.pressZ;
    calculatePitchHz();
    age++;
}

void MPEVoice::reset() {
    memset(&st, 0, sizeof(st));
    memset(&tg, 0, sizeof(tg));
    age = 0;
}

unsigned int MPEVoice::getAge() {
    return age;
}

const MPEVoiceState & MPEVoice::getState() {
    return st;
}

}