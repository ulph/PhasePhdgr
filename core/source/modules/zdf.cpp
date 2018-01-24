#include "zdf.hpp"
#include "inlines.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>

inline float normalizeFrequency(float f, float fsInv) {
    float s = f*fsInv;
    assert(s <= 0.5f);
    assert(s >= 0.0f);
    return f*fsInv*2.0f*(float)M_PI;
}

inline float prewarp(float wc) {
    assert(wc >= 0.0f);
    assert(wc <= (float)M_PI);
    wc = 2.0f * tanf(wc * 0.5f);
//    wc = wc > (float)M_PI ? (float)M_PI : wc;
//    wc = wc < 0.0f ? 0.0f : wc;
    return wc;
}

inline float designZdf1pLpGain(float wc) {
    assert(wc >= 0.0f);
    assert(wc <= (float)M_PI);
    wc = prewarp(wc);
    return wc * 0.5f;
}

inline float calculateZdf1pLp(float x, float z1, float g, float& v) {
    v = (x - z1) * g / (1.0f + g);
    return v + z1;
}

inline float updateZdf1pLpState(float y, float v) {
    return y + v;
}

inline float lowShelfCutoff(float wmid, float k) {
    float arg = (1.0f + k);
    float denom = sqrt(arg);
    return wmid / denom;
}

inline float highShelfCutoff(float wmid, float k) {
    float arg = (1.0f + k);
    return wmid * sqrt(arg);
}

Zdf1p::Zdf1p() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("fc", 16000.f, "Hz"));
    outputs.push_back(Pad("low"));
    outputs.push_back(Pad("high"));
    outputs.push_back(Pad("all"));
}

void Zdf1p::process() {
    float x = inputs[0].value;
    float fc = limit(inputs[1].value, 1.0f, fs*0.5f);
    float wc = normalizeFrequency(fc, fsInv);
    float g = designZdf1pLpGain(wc);

    float v = 0.0f;
    float y = calculateZdf1pLp(x, s, g, v);
    s = updateZdf1pLpState(y, v);

    outputs[0].value = y;
    outputs[1].value = x - y;
    outputs[2].value = y - (x - y);
}

// ...

Zdf1pLowShelf::Zdf1pLowShelf() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("fc", 16000.f, "Hz"));
    inputs.push_back(Pad("drop", 0.5f));
    outputs.push_back(Pad("out"));
}

void Zdf1pLowShelf::process() {
    float x = inputs[0].value;
    float fc = limit(inputs[1].value, 1.0f, fs*0.5f);
    float drop = limit(inputs[2].value, 0.0f, 1.0f);
    float k = -drop;

    float wm = normalizeFrequency(fc, fsInv);

    float wcl = lowShelfCutoff(wm, k);
    wcl = wcl > wm ? wm : wcl;
    float gl = designZdf1pLpGain(wcl);

    float vl = 0.0f;
    float yl = calculateZdf1pLp(x, s, gl, vl);
    s = updateZdf1pLpState(yl, vl);

    outputs[0].value = x + k * yl;
}

Zdf1pHighShelf::Zdf1pHighShelf() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("fc", 16000.f, "Hz"));
    inputs.push_back(Pad("drop", 0.5f));
    outputs.push_back(Pad("out"));
}

void Zdf1pHighShelf::process() {
    float x = inputs[0].value;
    float fc = limit(inputs[1].value, 1.0f, fs*0.5f);
    float drop = limit(inputs[2].value, 0.0f, 1.0f);
    float k = -drop;

    float wm = normalizeFrequency(fc, fsInv);

    float wch = highShelfCutoff(wm, k);
    wch = wch < wm ? wm : wch;
    float gh = designZdf1pLpGain(wch);

    float vh = 0.0f;
    float yh = calculateZdf1pLp(x, s, gh, vh);
    s = updateZdf1pLpState(yh, vh);
    float yh_lp = x - yh;

    outputs[0].value = x + k * yh_lp;
}

Zdf4pLadder::Zdf4pLadder() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("fc", 16000.f, "Hz"));
    inputs.push_back(Pad("res"));
    inputs.push_back(Pad("fbHpFc", 10.f, "Hz"));
    outputs.push_back(Pad("out1p"));
    outputs.push_back(Pad("out2p"));
    outputs.push_back(Pad("out3p"));
    outputs.push_back(Pad("out4p"));
}

void Zdf4pLadder::process() {
    float x = inputs[0].value;
    float k = limit(inputs[2].value, 0.0f, 2.0f) * 4.0f;

    float fc = limit(inputs[1].value, 1.0f, fs*0.5f);
    float wc = normalizeFrequency(fc, fsInv);
    float g = designZdf1pLpGain(wc);

    // feedback

    float G = g*g*g*g;
    float S = g*g*g*s1 + g*g*s2 + g*s3 + s4;

    // high-pass of feedback section
    float fbHpFc = limit(inputs[3].value, 1.0f, fs*0.5f);
    float fbHpWc = normalizeFrequency(fbHpFc, fsInv);
    float h = designZdf1pLpGain(fbHpWc);

    float vS = 0.0f;
    float Slp = calculateZdf1pLp(S, S5, h, vS);
    S5 = updateZdf1pLpState(Slp, vS);
    S -= Slp;
    
    // Saturation
    S = tanhf(S); // feedback

    float u = (x - k*S) / (1 + k*G);
    u = tanhf(u); // input + feedback

    // LP sections

    float v1 = 0.0f;
    float y1 = calculateZdf1pLp(u, s1, g, v1);
    s1 = updateZdf1pLpState(y1, v1);
    outputs[0].value = y1;

    float v2 = 0.0f;
    float y2 = calculateZdf1pLp(y1, s2, g, v2);
    s2 = updateZdf1pLpState(y2, v2);
    outputs[1].value = y2;

    float v3 = 0.0f;
    float y3 = calculateZdf1pLp(y2, s3, g, v3);
    s3 = updateZdf1pLpState(y3, v3);
    outputs[2].value = y3;

    float v4 = 0.0f;
    float y4 = calculateZdf1pLp(y3, s4, g, v4);
    s4 = updateZdf1pLpState(y4, v4);
    outputs[3].value = y4;

}
