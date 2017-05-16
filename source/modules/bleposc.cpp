#include "bleposc.hpp"
#include <string.h>
#include "sinc.hpp"
#include "inlines.hpp"
#include "rlc.hpp"

const int c_numFraction = 1000;
const auto c_blitTable = FractionalSincTable(BlitOsc::c_blitN, c_numFraction, (float)M_PI, true);
// TODO; investigate something like fs/4 bandlimit on the sinc itself...

BlitOsc::BlitOsc()
    : buf{0.f}
    , blit{0.f}
    , bufPos(0)
    , cumSum(0.f)
    , stage(0)
    , internalPhase(0.f)
    , sync(0.f)
    , last_nFreq(0.f)
    , last_pwm(0.f)
    , last_shape(0.f)
    , last_cumSum(0.f)
{
    inputs.push_back(Pad("freq"));
    inputs.push_back(Pad("shape")); // 2saw <-> saw <-> square ... (a 'serendipity' with 2saw, cool stuff)
    inputs.push_back(Pad("pwm")); // pulse/saw <-> square/2saw <-> pulse/saw ...
    inputs.push_back(Pad("reset")); // for osc sync
    inputs.push_back(Pad("freq_drift", 0.1f));
    inputs.push_back(Pad("amp_drift", 0.1f));
    outputs.push_back(Pad("output"));
}

void BlitOsc::process(uint32_t fs)
{
    const float c_slew = 0.9f; // a bit arbitrary ... and not sure how much this adds as stuff should actually be smoothed before

    float freq = limit(inputs[0].value, 0.0f, float(fs)*0.5f);
    float shape = limit(inputs[1].value);
    float pwm = limit(inputs[2].value, 0.0f, 1.0f); // TODO weird shit happens on -1 when syncing
    float newSync = inputs[3].value;

    // TODO, slowly random walk within some range, or randomly perturb (slewed, mind) each iteration ...
    float freq_drift = inputs[4].value;
    float amp_drift = inputs[4].value;
    // or, make a nice breakout box for this?

    float nFreq = 2.f*freq/(float)fs; // TODO get this from inBus wall directly

    // slew stuff -- workaround for jittery automation
    nFreq = c_slew*last_nFreq + (1-c_slew)*nFreq;
    pwm = c_slew*last_pwm + (1-c_slew)*pwm;
    shape = c_slew*last_shape + (1-c_slew)*shape;

    float bias = nFreq;

    if(nFreq == 0) return;

    // TODO, bias should also be integrated from some (perhaps order 1 is sufficient) derivative of freq
    // TODO, triangles (using derivative...)

    float leak = 1.f-nFreq*0.1; // maybe leak here can be somewhat adjustable?

    if(newSync > 0 && sync <= 0){
        float syncFraction = (0 - sync) / (newSync - sync);
        // TODO - softsync
        float t = -1.0f; // target value
        float r = cumSum; // current value
        // accumulate future value
        for(int n=0; n<c_blitN; ++n){
            r += buf[(bufPos+n)%c_blitN];
        }
        c_blitTable.getCoefficients(syncFraction, &blit[0], c_blitN);
        for(int n=0; n<c_blitN; ++n){
            buf[(bufPos+n)%c_blitN] += (t-r)*blit[n];
        }
        internalPhase = -1.f + (1.f-syncFraction)*nFreq;
        stage = 0;
    }
    else{
        internalPhase += nFreq;
    }

    while(true){
        if(stage==0){
            if(internalPhase <= pwm) break;
            float fraction = (pwm - (internalPhase-nFreq)) / nFreq;
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] += 2.f*shape*blit[n];
            }
            stage=1;
        }
        if(stage==1){
            if(internalPhase <= 1.0f) break;
            float fraction = (1.f - (internalPhase-nFreq)) / nFreq;
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] -= 2.f*blit[n];
            }
            stage=0;
            internalPhase -= 2.0f;
        }
    }

    sync = newSync;

    last_cumSum = cumSum; // x
    cumSum = cumSum*leak + buf[bufPos] + (1-shape)*bias; // x+1

    outputs[0].value = CalcRcHp(cumSum, last_cumSum, outputs[0].value, freq*0.5f, fs); // cheat a bit and add a high-pass... removes some modulation gunk so sounds a bit nicer I think
    buf[bufPos] = 0.f;
    bufPos++;
    bufPos %= c_blitN;

    last_pwm = pwm;
    last_shape = shape;
    last_nFreq = nFreq;
}
