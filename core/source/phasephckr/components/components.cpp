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
            {"delayLeftTime", "MULADD"},
            {"delayRightTime", "MULADD"},
            {"leftDelayLP", "D_LP"},
            {"rightDelayLP", "D_LP"},
            {"leftDelayHP", "D_HP"},
            {"rightDelayHP", "D_HP"},
            {"saturation", "SNATAN"},
        },
        vector<ModulePortConnection>{
            {{"inBus", "left"}, {"leftDelay", "in"}},
            {{"inBus", "right"}, {"rightDelay", "in"}},

            {{"inBus", "leftTime"}, {"delayLeftTime", "add"}},
            {{"inBus", "rightTime"}, {"delayRightTime", "add"}},
            {{"inBus", "leftModDepth"}, {"delayLeftTime", "mul"}},
            {{"inBus", "rightModDepth"}, {"delayRightTime", "mul"}},

            {{"inBus", "leftHpHz"}, {"leftDelayHP", "wc"}},
            {{"inBus", "rightHpHz"}, {"rightDelayHP", "wc"}},
            {{"inBus", "leftLpHz"}, {"leftDelayLP", "wc"}},
            {{"inBus", "rightLpHz"}, {"rightDelayLP", "wc"}},

            {{"inBus", "feedback"}, {"feedbackGain", "gain"}},
            {{"inBus", "modHz"}, {"lfoPhase", "freq"}},
            {{"inBus", "saturation"}, {"saturation", "prescaler"}},

            {{"leftDelay", "out"}, {"saturation", "left"}},
            {{"rightDelay", "out"}, {"saturation", "right"}},
            {{"saturation", "left"}, {"leftDelayHP", "in"}},
            {{"saturation", "right"}, {"rightDelayHP", "in"}},
            {{"leftDelayHP", "out"}, {"leftDelayLP", "in"}},
            {{"rightDelayHP", "out"}, {"rightDelayLP", "in"}},

            {{"leftDelayLP", "out"}, {"feedbackGain", "left"}},
            {{"rightDelayLP", "out"}, {"feedbackGain", "right"}},
            {{"feedbackGain", "left"}, {"rightDelay", "in"}},
            {{"feedbackGain", "right"}, {"leftDelay", "in"}},

            {{"lfoPhase", "phase"}, {"lfo", "phase"}},
            {{"lfo", "sine"}, {"delayLeftTime", "in"}},
            {{"lfo", "sine"}, {"delayRightTime", "in"}},
            {{"delayLeftTime", "out"}, {"leftDelay", "time"}},
            {{"delayRightTime", "out"}, {"rightDelay", "time"}},

            {{"leftDelayLP", "out"}, {"outBus", "left"}},
            {{"rightDelayLP", "out"}, {"outBus", "right"}},
        },
        std::map<ModulePort, float>{
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
            {"env", "CAMELENV"}
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
        std::map<ModulePort, float>{
            {{"env", "onBumpHeight"}, 1.0f},
            {{"env", "offBumpHeight"}, 0.0f},
            {{"env", "offAttackSpeed"}, 0.0f },
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
