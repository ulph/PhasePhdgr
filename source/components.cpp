#include "components.hpp"
#include "design.hpp"
#include <assert.h>

using namespace std;

namespace PhasePhckr{

const ComponentDescriptor stereoTape = {
    vector<PadDescription>{
        {"left", "", 0.f},
        {"right", "", 0.f},
        /*
        {"leftTime", {{"delayLeftTime", "shift"}}},
        {"rightTime", {{"delayRightTime", "shift"}}},
        {"leftModDepth", {{"delayLeftTime", "scale"}}},
        {"rightModDepth", {{"delayRightTime", "scale"}}},
        {"feedback", {{"feedbackGain", "gain"}}},
        {"leftHpHz", {{"leftDelayHP", "wc"}}},
        {"rightHpHz", {{"rightDelayHP", "wc"}}},
        {"leftLpHz", {{"leftDelayLP", "wc"}}},
        {"rightLpHz", {{"rightDelayLP", "wc"}}},
        {"modHz", {{"lfoPhase", "freq"}}},
        {"saturation", {{"saturation", "prescaler"}}}
        */
    },
    vector<PadDescription>{
        {"left", "", 0.f},
        {"right", "", 0.f},
    },
    ConnectionGraphDescriptor{
        vector<ModuleVariable>{
            {"leftDelay", "DELAY"},
            {"rightDelay", "DELAY"},
            {"lfoPhase", "PHASE"},
            {"lfo", "SINE"},
            {"feedbackGain", "GAIN"},
            {"delayLeftTime", "SCLSHFT"},
            {"delayRightTime", "SCLSHFT"},
            {"leftDelayLP", "RCLP"},
            {"rightDelayLP", "RCLP"},
            {"leftDelayHP", "RCHP"},
            {"rightDelayHP", "RCHP"},
            {"saturation", "SSATAN"},
        },
        vector<ModulePortConnection>{
            {{"inBus", "left"}, {"leftDelay", "in"}},
            {{"inBus", "right"}, {"rightDelay", "out"}},
            // ...

            {{"leftDelay", "out"}, {"saturation", "left"}},
            {{"rightDelay", "out"}, {"saturation", "right"}},
            {{"saturation", "left"}, {"leftDelayHP", "x1"}},
            {{"saturation", "right"}, {"rightDelayHP", "x1"}},
            {{"leftDelayHP", "y1"}, {"leftDelayLP", "x1"}},
            {{"rightDelayHP", "y1"}, {"rightDelayLP", "x1"}},

            {{"leftDelayLP", "y1"}, {"feedbackGain", "left"}},
            {{"rightDelayLP", "y1"}, {"feedbackGain", "right"}},
            {{"feedbackGain", "left"}, {"rightDelay", "in"}},
            {{"feedbackGain", "right"}, {"leftDelay", "in"}},

            {{"lfoPhase", "phase"}, {"lfo", "phase"}},
            {{"lfo", "sine"}, {"delayLeftTime", "input"}},
            {{"lfo", "sine"}, {"delayRightTime", "input"}},
            {{"delayLeftTime", "output"}, {"leftDelay", "time"}},
            {{"delayRightTime", "output"}, {"rightDelay", "time"}},

            {{"leftDelayLP", "y1"}, {"outBus", "left"}},
            {{"rightDelayLP", "y1"}, {"outBus", "right"}},
        },
        vector<ModulePortValue>{
            {"lfoPhase", "freq", 2.0f},
            {"delayLeftTime", "shift", 0.22f},
            {"delayRightTime", "shift", 0.45f},
            {"delayLeftTime", "scale", 0.001f},
            {"delayRightTime", "scale", 0.001f},
            {"leftDelayLP", "wc", 8500.0f},
            {"rightDelayLP", "wc", 7500.0f},
            {"leftDelayHP", "wc", 350.0f},
            {"rightDelayHP", "wc", 450.0f},
            {"feedbackGain", "gain", 0.5f},
        },
    },
    string{"Time modulated stereo cross feedback delay with saturating filter stages."}
};

/*
const ComponentDescriptor adsr = {
    vector<ModulePortAlias>{
        {"gate", {{"env", "gate"}}},
        {"A", {{"env", "onAttackSpeed"}}},
        {"D", {{"env", "onDecaySpeed"}}},
        {"S", {{"env", "sustainHeight"}}},
        {"R", {{"env", "offDecaySpeed"}}},
        {"APow", {{"env", "onAttackPow"}}},
        {"DPow", {{"env", "onDecayPow"}}},
        {"RPow", {{"env", "offDecayPow"}}},
    },
    vector<ModulePortAlias>{
        {"value", {{"env", "value"}}}
    },
    ConnectionGraphDescriptor{
        vector<ModuleVariable>{
            {"env", "ENV"}
        },
        vector<ModulePortConnection>{},
        vector<ModulePortValue>{
            {"env", "onBumpHeight", 1.0f},
            {"env", "offBumpHeight", 0.0f},
            {"env", "offAttackSpeed", 0.0f},
        }
    },
    string("ADSR envelope with shape control.\n(Simplification of ENV module)")
};

const ComponentDescriptor lfo = {
    vector<ModulePortAlias>{
        {"freq", {{"phase", "freq"}}},
        {"gain", {{"gain", "in2"}}},
        {"trig", {{"phase", "trig"}}}
    },
    vector<ModulePortAlias>{
        {"value", {{ "gain", "prod"}}}
    },
    ConnectionGraphDescriptor{
        vector<ModuleVariable>{
            {"phase", "PHASE"},
            {"sine", "SINE"},
            {"gain", "MUL"},
            {"lin", "SCLSHFT"}
        },
        vector<ModulePortConnection>{
            {"phase", "phase", "sine", "phase"},
            {"sine", "sine", "lin", "input"},
            {"lin", "output", "gain", "in1"},
        },
        vector<ModulePortValue>{
            {"lin", "scale", 0.5f},
            {"lin", "shift", 0.5f},
            {"gain", "in2", 1.0f},
        }
    },
    string("Convinience (simple sine) LFO bundle.")
};


const ComponentDescriptor chorus = {
    vector<ModulePortAlias>{
        {"mono", {{"monoIn", "value"}}},
        {"freq", {{"lfoHz", "value"}}},
        {"depth",{{"lfoGain", "value"}}},
    },
    vector<ModulePortAlias>{
        {"left", {{"dl", "out"}}},
        {"right",{{"dr", "out"}}}
    },
    ConnectionGraphDescriptor{
        vector<ModuleVariable>{
            {"lfoGain", "CONST"},
            {"lfoHz", "CONST"},
            {"lfoL", "@LFO"},
            {"lfoR", "@LFO"},
            {"monoIn", "CONST"},
            {"lrDiff", "CONST"},
            {"timeScale", "GAIN"},
            {"dl", "DELAY"},
            {"dr", "DELAY"},
            // todo, will need more stages -- or maybe a feedback loop would work?
        },
        vector<ModulePortConnection>{
            {"monoIn", "value", "dl", "in"},
            {"monoIn", "value", "dr", "in"},

            {"lfoGain", "value", "lfoL", "gain"},
            {"lfoGain", "value", "lfoR", "gain"},

            {"lfoHz", "value", "lfoL", "freq"},
            {"lfoHz", "value", "lfoR", "freq"},
            {"lrDiff", "value", "lfoR", "freq"},
            
            {"lfoL", "value", "timeScale", "left"},
            {"lfoR", "value", "timeScale", "right"},

            {"timeScale", "left", "dl", "time"},
            {"timeScale", "right", "dr", "time"},
        },
        vector<ModulePortValue>{ 
            {"timeScale", "gain", 0.01f},
            {"dl", "time", 0.0f},
            {"dr", "time", 0.0f},
            {"lfoHz", "value", 0.1f},
            {"lfoGain", "value", 1.0f},
            {"lrDiff", "value", 0.1f},
        },
    },
    string("Simple chorus.")
};
*/

void ComponentRegister::registerFactoryComponents() {
    registerComponent("@STEREOTAPE", stereoTape);
//    registerComponent("@ADSR", adsr);
//    registerComponent("@LFO", lfo);
//    registerComponent("@CHORUS", chorus);
    // etc
}

bool ComponentRegister::registerComponent(string name, const ComponentDescriptor & desc) {
    if (r.count(name)) return false;
    r[name] = desc;
    return true;
}

bool ComponentRegister::getComponent(string name, ComponentDescriptor & desc) const {
    if (!r.count(name)) return false;
    desc = r.at(name);
    return true;
}

void ComponentRegister::makeComponentDoc(const string &type, const ComponentDescriptor & cmp, ModuleDoc &doc) {
    doc.type = type;
    doc.inputs = cmp.inBus;
    doc.outputs = cmp.outBus;
    doc.docString = cmp.docString;
}

bool ComponentRegister::makeComponentDoc(const string &type, ModuleDoc &doc) const {
    const auto it = r.find(type);
    if (it == r.end()) { return false; }
    const auto& cmp = it->second;
    makeComponentDoc(type, cmp, doc);
    return true;
}

void ComponentRegister::makeComponentDocs(Doc& doc) const {
    for (const auto kv : r) {
        ModuleDoc md;
        if (makeComponentDoc(kv.first, md)) {
            doc.add(md);
        }
    }
}

}
