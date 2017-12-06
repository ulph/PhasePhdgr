#include "phasephckr/components.hpp"

#include <assert.h>

using namespace std;

namespace PhasePhckr{

const ComponentDescriptor stereoTape = {
    vector<PadDescription>{
        {"left", "", 0.f},
        {"right", "", 0.f},
        {"leftTime", "", 0.22f},
        {"rightTime", "", 0.45f},
        {"leftModDepth", "", 0.001f},
        {"rightModDepth", "", 0.001f},
        {"feedback", "", 0.5f},
        {"leftHpHz", "", 350.f},
        {"rightHpHz", "", 450.f},
        {"leftLpHz", "", 8500.f},
        {"rightLpHz", "", 7500.f},
        {"modHz", "", 2.f},
        {"saturation", "", 1.f},
    },
    vector<PadDescription>{
        {"left", "", 0.f},
        {"right", "", 0.f},
    },
    ConnectionGraphDescriptor{
        map<string, string>{
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
            {{"inBus", "right"}, {"rightDelay", "in"}},

            {{"inBus", "leftTime"}, {"delayLeftTime", "shift"}},
            {{"inBus", "rightTime"}, {"delayRightTime", "shift"}},
            {{"inBus", "leftModDepth"}, {"delayLeftTime", "scale"}},
            {{"inBus", "rightModDepth"}, {"delayRightTime", "scale"}},

            {{"inBus", "leftHpHz"}, {"leftDelayHP", "wc"}},
            {{"inBus", "rightHpHz"}, {"rightDelayHP", "wc"}},
            {{"inBus", "leftLpHz"}, {"leftDelayLP", "wc"}},
            {{"inBus", "rightLpHz"}, {"rightDelayLP", "wc"}},

            {{"inBus", "feedback"}, {"feedbackGain", "gain"}},
            {{"inBus", "modHz"}, {"lfoPhase", "freq"}},
            {{"inBus", "saturation"}, {"saturation", "prescaler"}},

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
            // any "api"-facing values are better set on inBus
        },
    },
    string{"Time modulated stereo cross feedback delay with saturating filter stages."}
};

const ComponentDescriptor adsr = {
    vector<PadDescription>{
        {"gate", "", 0.f},
        {"A", "", 0.025f},
        {"D", "", 0.1f},
        {"S", "", 0.5f},
        {"R", "", 0.25f},
        {"APow", "", 0.5f},
        {"DPow", "", 2.f},
        {"RPow", "", 4.f},
    },
    vector<PadDescription>{
        {"value", "", 0.f}
    },
    ConnectionGraphDescriptor{
        map<string, string>{
            {"env", "ENV"}
        },
        vector<ModulePortConnection>{
            {{"inBus", "gate"}, {"env", "gate"}},
            {{"inBus", "A"}, {"env", "onAttackSpeed"}},
            {{"inBus", "D"}, {"env", "onDecaySpeed"}},
            {{"inBus", "S"}, {"env", "sustainHeight"}},
            {{"inBus", "R"}, {"env", "offDecaySpeed"}},
            {{"inBus", "APow"}, {"env", "onAttackPow"}},
            {{"inBus", "DPow"}, {"env", "onDecayPow"}},
            {{"inBus", "RPow"}, {"env", "offDecayPow"}},
            {{"env", "value"}, {"outBus", "value"}},
        },
        vector<ModulePortValue>{
            {"env", "onBumpHeight", 1.0f},
            {"env", "offBumpHeight", 0.0f},
            {"env", "offAttackSpeed", 0.0f},
        }
    },
    string("ADSR envelope with shape control.\n(Simplification of ENV module)")
};

void ComponentRegister::registerFactoryComponents() {
    registerComponent("@STEREOTAPE", stereoTape);
    registerComponent("@ADSR", adsr);
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
