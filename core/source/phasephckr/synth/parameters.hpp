#pragma once

namespace PhasePhckr {

    const float c_slewFactor = 0.995f;

    struct GlobalTimeDataState {
        GlobalTimeDataState()
            : nominator(4)
            , denominator(4)
            , bpm(120)
            , position(0)
            , time(0)
        {}
        int nominator;
        int denominator;
        float bpm;
        float position;
        float time;
    };

    struct GlobalDataState {
        GlobalDataState() 
            : exp(0)
            , brt(0)
            , mod(0) 
        {}
        float exp;
        float brt;
        float mod;
    };

    class GlobalData {
    private:
        GlobalTimeDataState timeSt;
        GlobalDataState tg;
        GlobalDataState st;
        float slewFactor;
    public:
        void modwheel(float v) { tg.mod = v; }
        void expression(float v) { tg.exp = v; }
        void breath(float v) { tg.brt = v; }
        void signature(int num, int den) { timeSt.nominator = num; timeSt.denominator = den; }
        void bpm(float bpm) { timeSt.bpm = bpm; }
        void position(float pos) { timeSt.position = pos; }
        void time(float t) { timeSt.time = t; }
        GlobalData() : slewFactor(c_slewFactor) {}
        void update() {
            st.mod = slewFactor * st.mod + (1.0f - slewFactor) * tg.mod;
            st.exp = slewFactor * st.exp + (1.0f - slewFactor) * tg.exp;
            st.brt = slewFactor * st.brt + (1.0f - slewFactor) * tg.brt;
        }
        const GlobalDataState & getState() const { return st; }
        const GlobalTimeDataState & getTimeState() const { return timeSt; }
    };

    struct MPEVoiceState {
        MPEVoiceState()
            : strikeZ(0)
            , pressZ(0)
            , liftZ(0)
            , slideY(0)
            , glideX(0)
            , pitchHz(0)
            , gate(0)
        {}
        float strikeZ; // velocity, 0 to 1
        float pressZ; // aftertouch, 0 to 1
        float liftZ; // release velocity, 0 to 1
        float slideY; // horizontal up/down, 0 to 1
        float glideX; // pitch bend, -1 to 1
        float pitchHz; // pitch in Hz, combination of root note and pitch bend with bend ranges taken into account
        float gate; // open or closed, for statey stuff
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

    class MPEVoice {
    public:
        MPEVoice();
        void on(int note, float velocity);
        void off(int note, float velocity);
        void glide(float glide);
        void slide(float slide);
        void press(float press);
        void update();
        void reset();
        const MPEVoiceState & getState();
        unsigned int getAge();
    private:
        MPEVoiceState tg; // target, so we can slew x,y,z
        MPEVoiceState st; // state
        MPEVoiceConfig cfg;
        float slewFactor;
        int rootNote;
        unsigned int age;
        void calculatePitchHz();
    };

}
