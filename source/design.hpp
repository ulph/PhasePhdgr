#pragma once

#include <utility>
#include <vector>
#include <map>
#include <iostream>

#include "connectiongraph.hpp"

namespace PhasePhckr {

struct ModuleVariable {
    std::string name;
    std::string type;
};

struct ModulePort {
    std::string module;
    std::string port;
};

struct ModulePortConnection {
    ModulePort from;
    ModulePort to;
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

// populate a connectionGraph
std::map<std::string, int> DesignConnectionGraph(
    ConnectionGraph &connectionGraph,
    const ConnectionGraphDescriptor &description
);

// the following two are for creating "components"
struct BusDiscriptor {
    std::string type; // 'input' or 'output'
    ModulePort value;
};

struct ComponentDescriptor {
    std::vector<BusDiscriptor> bus;
    ConnectionGraphDescriptor graph;
};

}
