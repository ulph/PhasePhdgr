#include "components.hpp"
#include "design.hpp"

namespace PhasePhckr{

const ComponentDescriptor stereoTape = {
    std::vector<ModulePortAlias>{
        {"left", {"leftDelay", "in"}},
        {"right", {"rightDelay", "in"}},
        {"leftTime", {"delayLeftTime", "shift"}},
        {"rightTime", {"delayRightTime", "shift"}},
        {"leftModDepth", {"delayLeftTime", "scale"}},
        {"rightModDepth", {"delayRightTime", "scale"}},
        {"feedback", {"feedbackGain", "gain"}},
        {"leftHpHz", {"leftDelayHP", "wc"}},
        {"rightHpHz", {"rightDelayHP", "wc"}},
        {"leftLpHz", {"leftDelayLP", "wc"}},
        {"rightLpHz", {"rightDelayLP", "wc"}},
        {"modHz", {"lfoPhase", "freq"}},
        {"saturation", {"saturation", "prescaler"}}
    },
    std::vector<ModulePortAlias>{
        {"left", {"leftDelayLP", "y1"}},
        {"right", {"rightDelayLP", "y1"}}
    },
    ConnectionGraphDescriptor{
        std::vector<ModuleVariable>{
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
        std::vector<ModulePortConnection>{
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
            {{"delayRightTime", "output"}, {"rightDelay", "time"}}
        },
        std::vector<ModulePortValue>{
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
    std::string{"Time modulated stereo cross feedback delay with saturating filter stages."}
};

void ComponentRegister::registerFactoryComponents() {
    registerComponent("@STEREOTAPE", stereoTape);
    // etc
}

bool ComponentRegister::registerComponent(std::string name, const ComponentDescriptor & desc) {
    if (r.count(name)) return false;
    r[name] = desc;
    return true;
}

bool ComponentRegister::getComponent(std::string name, ComponentDescriptor & desc) const {
    if (!r.count(name)) return false;
    desc = r.at(name);
    return true;
}

void ComponentRegister::makeComponentDocs(std::vector<ModuleDoc> &docList) const {
    for (const auto kv : r) {
        ModuleDoc doc;
        doc.type = kv.first;
        for (const auto i : kv.second.inputs) {
            PadDescription pd;
            pd.name = i.alias;
            doc.inputs.emplace_back(pd);
        }
        for (const auto o : kv.second.outputs) {
            PadDescription pd;
            pd.name = o.alias;
            doc.outputs.emplace_back(pd);
        }
        doc.docString = kv.second.docString;
        docList.emplace_back(doc);
    }
}

}