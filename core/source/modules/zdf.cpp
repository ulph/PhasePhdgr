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
    return 2.0f * tanf(wc * 0.5f);
}

inline float designZdf1pLpGain(float wc) {
    assert(wc >= 0.0f);
    assert(wc <= (float)M_PI);
    return prewarp(wc) * 0.5f;
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
    float y = calculateZdf1pLp(x, z1, g, v);
    z1 = updateZdf1pLpState(y, v);

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
    float yl = calculateZdf1pLp(x, z1l, gl, vl);
    z1l = updateZdf1pLpState(yl, vl);

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
    float yh = calculateZdf1pLp(x, z1h, gh, vh);
    z1h = updateZdf1pLpState(yh, vh);
    float yh_lp = x - yh;

    outputs[0].value = x + k * yh_lp;
}