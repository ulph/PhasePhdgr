#pragma once

#include "PhasePhckr.h"

namespace PhasePhckr {

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
        GlobalDataState tg;
        GlobalDataState st;
        float slewFactor;
    public:
        void modwheel(float v) { tg.mod = v; }
        void expression(float v) { tg.exp = v; }
        void breath(float v) { tg.brt = v; }
        GlobalData() : slewFactor(c_slewFactor) {}
        void update() {
            st.mod = slewFactor * st.mod + (1.0f - slewFactor) * tg.mod;
            st.exp = slewFactor * st.exp + (1.0f - slewFactor) * tg.exp;
            st.brt = slewFactor * st.brt + (1.0f - slewFactor) * tg.brt;
        }
        const GlobalDataState & getState() const { return st; }
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
