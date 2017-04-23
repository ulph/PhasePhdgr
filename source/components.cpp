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
            {"rightDelayHP", "RCHP"}
        },
        std::vector<ModulePortConnection>{
            {{"leftDelay", "out"}, {"leftDelayHP", "x1"}},
            {{"rightDelay", "out"}, {"rightDelayHP", "x1"}},
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
            {"feedbackGain", "gain", 0.5f}
        }
    }
};

typedef std::map<std::string, ComponentDescriptor> ComponentRegistry_t;
ComponentRegistry_t *g_componentRegistry = nullptr;

static ComponentRegistry_t* getComponentRegistry() {
    if (!g_componentRegistry) {
        // singleton and register the factory stuff
        g_componentRegistry = new ComponentRegistry_t();
        (*g_componentRegistry)["@STEREOTAPE"] = stereoTape;
    };
    return g_componentRegistry;
}

bool registerComponent(std::string name, const ComponentDescriptor & desc) {
    // for registering any user stuff
    ComponentRegistry_t * r = getComponentRegistry();
    if (r->count(name)) return false;
    (*r)[name] = desc;
    return true;
}

bool getComponent(std::string name, ComponentDescriptor & desc) {
    ComponentRegistry_t * r = getComponentRegistry();
    if (!(r->count(name))) return false;
    const ComponentDescriptor & cmp = (*r)[name];
    desc = cmp;
    return true;
}

}