#pragma once

#include <utility>
#include <vector>
#include <map>
#include <iostream>

class ConnectionGraph;

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

// TODO, deprecate ModulePortConnection (1:1) in favour of ModulePortConnections (1:N)
struct ModulePortConnection {
    ModulePort source;
    ModulePort target;
};

struct ModulePortConnections {
    ModulePort source;
    std::vector<ModulePort> targets;
};

struct ModulePortTrivialConnection {
    std::string source;
    std::string target;
};

// TODO, Mechanisms for ModulePortTrivialConnection which implies 
//     "mono"->"mono", or "left"->"left"+"right"->"right", 
//     defaulting to 0->0 if nothing else works
//     ... this is such a common use case that it warrents the extra logic in code

struct ModulePortValue {
    ModulePort target;
    float value;
};

struct ConnectionGraphDescriptor {
    std::vector<ModuleVariable> modules;
    std::vector<ModulePortConnection> connections;
    std::vector<ModulePortValue> values;
};

void DesignConnectionGraph(
    ConnectionGraph &connectionGraph,
    ConnectionGraphDescriptor &description, // may be mutated inside
    std::map<std::string, int> & moduleHandles
);

/* Component (subgraph) stuff */

struct ModulePortAlias {
    std::string alias;
    ModulePort target;
};

struct ComponentDescriptor {
    std::vector<ModulePortAlias> inputs;
    std::vector<ModulePortAlias> outputs;
    ConnectionGraphDescriptor graph;
};

struct PatchDescriptor {
    ConnectionGraphDescriptor voiceGraph;
    ConnectionGraphDescriptor effectGraph;
};

}
