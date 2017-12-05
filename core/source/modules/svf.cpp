#include "svf.hpp"
#include <math.h>
#include "inlines.hpp"

ChamberlinFilter::ChamberlinFilter() 
    : band(0.f), low(0.f), high(0.f)
{
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("wc", 100.f));
    inputs.push_back(Pad("q"));
    outputs.push_back(Pad("low"));
    outputs.push_back(Pad("high"));
    outputs.push_back(Pad("band"));
    outputs.push_back(Pad("notch"));
}

void ChamberlinFilter::process() {
    const float stabF = 1.f / 6.f;
    float x = inputs[0].value;
    float wc = limit(inputs[1].value, 0.f, fs * stabF);
    float q = 1.f - limit(inputs[2].value, 0.f, 1.f);
    float f = 2.f * sinf((float)M_PI*wc / (float)fs);
    low += f * band;
    high = x - low - q*band;
    band += f*high;
    float notch = high + low;
    outputs[0].value = low;
    outputs[1].value = high;
    outputs[2].value = band;
    outputs[3].value = notch;
}

OpenChamberlinFilter::OpenChamberlinFilter()
{
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("wc", 100.f));
    inputs.push_back(Pad("q"));
    inputs.push_back(Pad("low"));
    inputs.push_back(Pad("high"));
    inputs.push_back(Pad("band"));
    outputs.push_back(Pad("low"));
    outputs.push_back(Pad("high"));
    outputs.push_back(Pad("band"));
    outputs.push_back(Pad("notch"));
}

void OpenChamberlinFilter::process() {
    const float stabF = 1.f / 6.f;
    float x = inputs[0].value;
    float wc = limit(inputs[1].value, 0.f, fs * stabF);
    float q = 1.f - limit(inputs[2].value, 0.f, 1.f);
    float f = 2.f * sinf((float)M_PI*wc / (float)fs);
    float low = inputs[3].value;
    float high = inputs[4].value;
    float band = inputs[5].value;
    low += f * band;
    high = x - low - q*band;
    band += f*high;
    float notch = high + low;
    outputs[0].value = low;
    outputs[1].value = high;
    outputs[2].value = band;
    outputs[3].value = notch;
}

static inline void TrapezoidalTanSVFDesign(float fc, float fsInv, float k, float &a1, float &a2, float &a3) {
    float g = tanf((float)M_PI*fc * fsInv);
    a1 = 1.f / (1.f + g*(g + k));
    a2 = g*a1;
    a3 = g*a2;
}

static inline void TrapezoidalTanSVFCalcStep(float v0, float a1, float a2, float a3, float ic1eq, float ic2eq, float &v1, float &v2) {
    float v3 = v0 - ic2eq;
    v1 = a1*ic1eq + a2*v3;
    v2 = ic2eq + a2*ic1eq + a3*v3;
}

static inline void TrapezoidalTanSVFGetOutput(float v0, float v1, float v2, float k, float &low, float &band, float &high, float &notch, float &peak, float&all) {
    low = v2;
    band = v1;
    high = v0 - k*v1 - v2;
    notch = low + high;
    peak = low - high;
    all = low + high - k*band;
}

static inline void TrapezoidalTanSVFUpdateState(float v1, float v2, float &ic1eq, float &ic2eq) {
    ic1eq = 2.0f * v1 - ic1eq;
    ic2eq = 2.0f * v2 - ic2eq;
}

static inline float TrapezoidalTanSVFGetK(float res) {
    return 2.0f - 2.0f * res;
}

static inline void TrapezoidalTanSVFInitPads(vector<Pad>& inputs, vector<Pad>& outputs) {
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("fc", 100.f));
    inputs.push_back(Pad("res"));

    outputs.push_back(Pad("low"));
    outputs.push_back(Pad("high"));
    outputs.push_back(Pad("band"));
    outputs.push_back(Pad("peak"));
    outputs.push_back(Pad("notch"));
    outputs.push_back(Pad("all"));
}

static inline void TrapezoidalTanSVFProcess(const vector<Pad>& inputs, vector<Pad>& outputs, float fs, float fsInv, float ic1eq, float ic2eq, float &v1, float &v2) {
    // get inputs from pads
    float v0 = inputs[0].value;
    float fc = limit(inputs[1].value, 0.0f, 0.5f*fs);
    float res = limit(inputs[2].value, 0.0f, 1.0f);
    float k = TrapezoidalTanSVFGetK(res);

    // design
    float a1, a2, a3;
    TrapezoidalTanSVFDesign(fc, fsInv, k, a1, a2, a3);

    // calculate one step
    TrapezoidalTanSVFCalcStep(v0, a1, a2, a3, ic1eq, ic2eq, v1, v2);

    // get outputs
    float low, band, high, notch, peak, all;
    TrapezoidalTanSVFGetOutput(v0, v1, v2, k, low, band, high, notch, peak, all);

    // store outputs on pads
    outputs[0].value = low;
    outputs[1].value = high;
    outputs[2].value = band;
    outputs[3].value = peak;
    outputs[4].value = notch;
    outputs[5].value = all;
}

TrapezoidalTanSVF::TrapezoidalTanSVF()
{
    TrapezoidalTanSVFInitPads(inputs, outputs);
}

void TrapezoidalTanSVF::process() {
    float band, low;
    TrapezoidalTanSVFProcess(inputs, outputs, fs, fsInv, ic1eq, ic2eq, band, low);
    TrapezoidalTanSVFUpdateState(band, low, ic1eq, ic2eq);
}

OpenTrapezoidalTanSVF::OpenTrapezoidalTanSVF()
{
    TrapezoidalTanSVFInitPads(inputs, outputs);
    inPadOffest = inputs.size();
    inputs.push_back(Pad("low"));
    inputs.push_back(Pad("band"));
}

void OpenTrapezoidalTanSVF::process() {
    float low  = inputs[inPadOffest +1].value;
    float band = inputs[inPadOffest +2].value;
    TrapezoidalTanSVFUpdateState(band, low, ic1eq, ic2eq);
    TrapezoidalTanSVFProcess(inputs, outputs, fs, fsInv, ic1eq, ic2eq, band, low);
}
