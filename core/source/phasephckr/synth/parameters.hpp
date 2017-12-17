#pragma once

#include "connectiongraph.hpp"

namespace PhasePhckr {

    const float c_slewFactor = 0.995f;

    struct GlobalTimeDataState {
        GlobalTimeDataState()
            : nominator(4)
            , denominator(4)
            , bpm(120)
            , barPosition(0)
            , position(0)
            , time(0)
        {}
        int nominator;
        int denominator;
        float bpm;
        float position;
        float barLength;
        float barPosition;
        float time;
    };

    struct GlobalDataState {
        GlobalDataState() 
            : exp{ 0.0f }
            , brt{ 0.0f }
            , mod{ 0.0f }
        {}
        float exp[ConnectionGraph::k_blockSize];
        float brt[ConnectionGraph::k_blockSize];
        float mod[ConnectionGraph::k_blockSize];
        float expTarget = 0.0f;
        float brtTarget = 0.0f;
        float modTarget = 0.0f;
        float a = c_slewFactor;
        void update() {
            const auto last = ConnectionGraph::k_blockSize - 1;
            exp[0] = a * exp[last] + (1.0f - a) * expTarget;
            brt[0] = a * brt[last] + (1.0f - a) * brtTarget;
            mod[0] = a * mod[last] + (1.0f - a) * modTarget;
            for (int i = 1; i < ConnectionGraph::k_blockSize; i++) {
                exp[i] = a * exp[i-1] + (1.0f - a) * expTarget;
                brt[i] = a * brt[i-1] + (1.0f - a) * brtTarget;
                mod[i] = a * mod[i-1] + (1.0f - a) * modTarget;
            }
        }
    };

    class GlobalData {
    private:
        GlobalTimeDataState timeSt;
        GlobalDataState st;
    public:
        void modwheel(float v) { st.modTarget = v; }
        void expression(float v) { st.expTarget = v; }
        void breath(float v) { st.brtTarget = v; }
        void signature(int num, int den) { 
            timeSt.nominator = num;
            timeSt.denominator = den;
            timeSt.barLength = 4.f*(float)num / float(den); 
        }
        void bpm(float bpm) { timeSt.bpm = bpm; }
        void position(float pos) { timeSt.position = pos; }
        void barPosition(float pos) { timeSt.barPosition = pos; }
        void time(float t) { timeSt.time = t; }
        void update() {
            // call only once per block
            st.update();
        }
        const GlobalDataState & getState() const { return st; }
        const GlobalTimeDataState & getTimeState() const { return timeSt; }
    };

    struct MPEVoiceState {
        float strikeZ = 0; // velocity, 0 to 1
        float pressZ[ConnectionGraph::k_blockSize] = { 0.0f }; // aftertouch, 0 to 1
        float pressZTarget = 0;
        float liftZ; // release velocity, 0 to 1
        float slideY[ConnectionGraph::k_blockSize] = { 0.0f }; // horizontal up/down, 0 to 1
        float slideYTarget = 0;
        float glideX[ConnectionGraph::k_blockSize] = { 0.0f }; // pitch bend, -1 to 1
        float glideXTarget = 0;
        float pitchHz[ConnectionGraph::k_blockSize] = { 0.0f }; // pitch in Hz, combination of root note and pitch bend with bend ranges taken into account
        float gate = 0; // open or closed, for statey stuff
        float noteIndex = 0; // which (normalized) note
        float voiceIndex = 0; // which voice
        float polyphony = 0; // how many voices
    };

    struct MPEVoiceConfig {
        float pitchRangeUp;
        float pitchRangeDown;
        // + any other config type of stuff like transpose, tuning etc.
        MPEVoiceConfig() 
            : pitchRangeUp(24)
            , pitchRangeDown(24)
        {}
    };

    static float NoteToHz(float note) {
        if (note > 127.f) {
            return 0;
        }
        float hz = 440.f * powf(2.f, (note - 69.f) / 12.f);
        return hz;
    }

    class MPEVoice {
    public:
        MPEVoice(){ reset(); }
        void on(int note, float velocity) {
            age = 0;
            st.gate = 1.f;
            rootNote = note;
            st.strikeZ = velocity;
            st.noteIndex = (float)note / 127.f;
        }
        void off(int note, float velocity) {
            if (note == rootNote && st.gate) {
                st.gate = 0.f;
                st.liftZ = velocity;
            }
        }
        void glide(float glide) { st.glideXTarget = glide; }
        void slide(float slide) { st.slideYTarget = slide; }
        void press(float press) { st.pressZTarget = press; }
        void update() {
            const auto last = ConnectionGraph::k_blockSize - 1;
            st.glideX[0] = a * st.glideX[last] + (1.0f - a) * st.glideXTarget;
            st.slideY[0] = a * st.slideY[last] + (1.0f - a) * st.slideYTarget;
            st.pressZ[0] = a * st.pressZ[last] + (1.0f - a) * st.pressZTarget;
            calculatePitchHz(0);
            for (int i = 1; i < ConnectionGraph::k_blockSize; i++) {
                st.glideX[i] = a * st.glideX[i-1] + (1.0f - a) * st.glideXTarget;
                st.slideY[i] = a * st.slideY[i-1] + (1.0f - a) * st.slideYTarget;
                st.pressZ[i] = a * st.pressZ[i-1] + (1.0f - a) * st.pressZTarget;
                calculatePitchHz(i);
            }
            if (age < UINT_MAX) {
                age++;
            }
        }
        void reset() {
            // retain
            auto vi = st.voiceIndex;
            auto p = st.polyphony;

            memset(&st, 0, sizeof(st));
            age = UINT_MAX;

            st.voiceIndex = vi;
            st.polyphony = p;
        }
        const MPEVoiceState & getState() const { return st; }
        unsigned int getAge() const { return age; }
        void setIndex(int index, int polyPhony) {
            st.voiceIndex = (float)index / float(polyPhony);
            st.polyphony = (float)polyPhony;
        }
    private:
        MPEVoiceState st;
        MPEVoiceConfig cfg;
        float a = c_slewFactor;
        int rootNote = 0;
        unsigned int age = UINT_MAX;
        void calculatePitchHz(int i) {
            float bendSemi = st.glideX > 0 ? st.glideX[i] * cfg.pitchRangeUp : st.glideX[i] * cfg.pitchRangeDown;
            st.pitchHz[i] = NoteToHz(rootNote + bendSemi);
        }
    };

}
