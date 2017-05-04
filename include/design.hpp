#pragma once

#include <utility>
#include <vector>
#include <map>
#include <iostream>

#include "components.hpp"

class ConnectionGraph;
class ComponentRegister;

namespace PhasePhckr {

/* Graph stuff */

struct ModuleVariable {
    std::string name;
    std::string type;
};

struct ModulePort {
    std::string module;
    std::string port;
};

struct ModulePortConnection {
    ModulePort source;
    ModulePort target;
};

struct ModulePortValue {
    ModulePort target;
    float value;
};

struct ConnectionGraphDescriptor {
    std::vector<ModuleVariable> modules;
    std::vector<ModulePortConnection> connections;
    std::vector<ModulePortValue> values;
};

/* Component (subgraph) stuff */

struct ModulePortAlias {
    std::string alias;
    ModulePort wrapped;
};

struct ComponentDescriptor {
    std::vector<ModulePortAlias> inputs;
    std::vector<ModulePortAlias> outputs;
    ConnectionGraphDescriptor graph;
    std::string docString;
};

/* Functions and aux types */

void designConnectionGraph(
    ConnectionGraph &connectionGraph,
    ConnectionGraphDescriptor &description,
    std::map<std::string, int> &moduleHandles,
    const ComponentRegister & cp
);


const ModuleVariable c_EffectInput = {"inBus", "_EFFECTINPUT"};
const ModuleVariable c_EffectOutput = {"outBus", "_EFFECTOUTPUT"};
const ModuleVariable c_VoiceInput = {"inBus", "_VOICEINPUT" };
const ModuleVariable c_VoiceOutput = {"outBus", "_VOICEOUTPUT"};

}
