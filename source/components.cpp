#include "components.hpp"
#include "design.hpp"

using namespace std;

namespace PhasePhckr{

const ComponentDescriptor stereoTape = {
    vector<ModulePortAlias>{
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
    vector<ModulePortAlias>{
        {"left", {"leftDelayLP", "y1"}},
        {"right", {"rightDelayLP", "y1"}}
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


const ComponentDescriptor adsr = {
    vector<ModulePortAlias>{
        {"gate", {"env", "gate"}},
        {"A", {"env", "onAttackSpeed"}},
        {"D", {"env", "onDecaySpeed"}},
        {"S", {"env", "sustainHeight"}},
        {"R", {"env", "offDecaySpeed"}},
        {"APow", {"env", "onAttackPow"}},
        {"DPow", {"env", "onDecayPow"}},
        {"RPow", {"env", "offDecayPow"}},
    },
    vector<ModulePortAlias>{
        {"value", {"env", "value"}}
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


void ComponentRegister::registerFactoryComponents() {
    registerComponent("@STEREOTAPE", stereoTape);
    registerComponent("@ADSR", adsr);
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
            pd.unit = "";
            pd.value = 0;
            for (const auto &mv : kv.second.graph.modules) {
                if (mv.name == i.wrapped.module) {
                    for (const auto &d : docList) {
                        if (d.type == mv.type) {
                            for (const auto &p : d.inputs) {
                                if (p.name == i.wrapped.port) {
                                    pd.unit = p.unit;
                                    pd.value = p.value;
                                }
                            }
                        }
                    }
                }
            }
            for (const auto &v : kv.second.graph.values) {
                if (v.target.module == i.wrapped.module) {
                    if (v.target.port == i.wrapped.port) {
                        pd.value = v.value;
                    }
                }
            }
            doc.inputs.emplace_back(pd);
        }
        for (const auto o : kv.second.outputs) {
            PadDescription pd;
            pd.name = o.alias;
            pd.unit = "";
            for (const auto &mv : kv.second.graph.modules) {
                if (mv.name == o.wrapped.module) {
                    for (const auto &d : docList) {
                        if (d.type == mv.type) {
                            for (const auto &p : d.outputs) {
                                if (p.name == o.wrapped.port) {
                                    pd.unit = p.unit;
                                }
                            }
                        }
                    }
                }
            }
            doc.outputs.emplace_back(pd);
        }
        doc.docString = kv.second.docString;
        docList.emplace_back(doc);
    }
}

}