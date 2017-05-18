#include "bleposc.hpp"
#include <string.h>
#include "sinc.hpp"
#include "inlines.hpp"
#include "rlc.hpp"

const int c_numFraction = 1000;
const auto c_blitTable = FractionalSincTable(BlitOsc::c_blitN, c_numFraction, (float)M_PI, true);
const auto c_dBlitTable = FractionalSincTable(BlitOsc::c_dBlitN, c_numFraction, (float)M_PI, true);

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
    , bias(0.f)
    , dBuf{0.f}
    , dBlit{0.f}
    , dBufPos(0.f)
{
    inputs.push_back(Pad("freq"));
    inputs.push_back(Pad("shape")); // triangle <-> saw <-> square
    inputs.push_back(Pad("pwm")); // assymetry, rather. does not affect a pure saw
    inputs.push_back(Pad("reset")); // for osc sync
    outputs.push_back(Pad("output"));
}

void BlitOsc::process(uint32_t fs)
{
    const float c_slew = 0.9f; // a bit arbitrary...

    float freq = limit(inputs[0].value, 0.0f, float(fs)*0.5f);
    float shape = limit(inputs[1].value);
    float pwm = limit(inputs[2].value, 0.0f, 1.0f); // TODO weird shit happens on -1 when syncing
    float newSync = inputs[3].value;

    float nFreq = 2.f*freq/(float)fs; // TODO get this from inBus wall directly

    // slew stuff -- workaround for jittery automation and simplistic aa
    nFreq = c_slew*last_nFreq + (1-c_slew)*nFreq;
    pwm = c_slew*last_pwm + (1-c_slew)*pwm;
    shape = c_slew*last_shape + (1-c_slew)*shape;

    bias = bias*0.99f + dBuf[dBufPos];
    dBuf[dBufPos] = 0.f;
    dBufPos++;
    dBufPos %= c_dBlitN;

    if(nFreq == 0) return; // nothing to do, just exit

    float upflank = limit(shape, 0.f, 1.f);
    float downflank = 1.f+limit(shape, -1.f, 0.f);

    /*
    bias = nFreq;
    // mix between triangle and saw shape
    bias = downflank*(1-upflank)*bias // mix in saw 'bias'
         + (1-downflank)*(stage-0.5)*bias*8; // or triangle 'bias'

    // slew bias a bit -- we should ideally integrate it, but hopefully we can get away with not having to
    const float c_bias_slew = 0.9;
    bias = last_bias*c_bias_slew + (1-c_bias_slew)*bias;
    last_bias = bias;
    */

    float leak = 1.f-nFreq*0.1; // make adjustable?

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
            float interval = (pwm - (internalPhase-nFreq));
            // as pwm can be modulated, we could potetionally end up (slightly) outside the range on higer pitches
            while(interval > 1.f) interval -= nFreq;
            while(interval < 0.f) interval += nFreq;
            float fraction = interval / nFreq;
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] += 2.f*upflank*blit[n];
            }
            stage=1;

            // catch-all
            c_dBlitTable.getCoefficients(fraction, &dBlit[0], c_dBlitN);
            for(int n=0; n<c_dBlitN; ++n){
                dBuf[(dBufPos+n)%c_dBlitN] += (1-shape)*nFreq*dBlit[n];
            }
        }

        if(stage==1){
            if(internalPhase <= 1.0f) break;
            float fraction = (1.f - (internalPhase-nFreq)) / nFreq;
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] -= 2.f*downflank*blit[n];
            }
            stage=0;
            internalPhase -= 2.0f;

            if(shape >= 0.f){
                // saw
            }
            else{
                // triangle
                c_dBlitTable.getCoefficients(fraction, &dBlit[0], c_dBlitN);
                for(int n=0; n<c_dBlitN; ++n){
                    dBuf[(dBufPos+n)%c_dBlitN] += shape*nFreq*dBlit[n]; // note, shape is negative
                }
            }
        }
    }

    sync = newSync;

    last_cumSum = cumSum; // x
    cumSum = cumSum*leak  // leaky integrate
            + buf[bufPos]  // with next value
            + bias; // add in the bias for triangle/square shape

    outputs[0].value = CalcRcHp(cumSum, last_cumSum, outputs[0].value, freq*0.5f, fs); // cheat a bit and add a high-pass... removes some modulation gunk so sounds a bit nicer I think
    buf[bufPos] = 0.f;
    bufPos++;
    bufPos %= c_blitN;

    last_pwm = pwm;
    last_shape = shape;
    last_nFreq = nFreq;
}
