#pragma once

#include <utility>
#include <vector>
#include <map>
#include <iostream>

class ConnectionGraph;

namespace PhasePhckr {

struct ModuleVariable {
    std::string name;
    std::string type;
};

template <typename T>
struct ModulePort_ {
    std::string module;
    T port;
};
typedef ModulePort_<std::string> ModulePort;
typedef ModulePort_<int> ModulePort_Numerical;

template <typename T>
struct ModulePortConnection_ {
    ModulePort_<T> from;
    ModulePort_<T> to;
};
typedef ModulePortConnection_<std::string> ModulePortConnection;
typedef ModulePortConnection_<int> ModulePortConnection_Numerical;

template <typename T>
struct ModulePortValue_ {
    ModulePort_<T> target;
    float value;
};
typedef ModulePortValue_<std::string> ModulePortValue;
typedef ModulePortValue_<int> ModulePortValue_Numerical;

struct ConnectionGraphDescriptor {
    std::vector<ModuleVariable> modules;
    std::vector<ModulePortConnection> connections;
    std::vector<ModulePortValue> values;
};

struct ConnectionGraphDescriptor_Numerical {
    std::vector<ModuleVariable> modules;
    std::vector<ModulePortConnection_Numerical> connections;
    std::vector<ModulePortValue_Numerical> values;
};

// populate a connectionGraph
std::map<std::string, int> DesignConnectionGraph(
    ConnectionGraph &connectionGraph,
    const ConnectionGraphDescriptor &description
);

std::map<std::string, int> DesignConnectionGraph(
    ConnectionGraph &connectionGraph,
    const ConnectionGraphDescriptor_Numerical &description
);

struct ComponentDescriptor {
    std::vector<ModulePort> input;
    std::vector<ModulePort> output;
    ConnectionGraphDescriptor graph;
};

struct PatchDescriptor {
    ConnectionGraphDescriptor voiceGraph;
    ConnectionGraphDescriptor effectGraph;
};

}
