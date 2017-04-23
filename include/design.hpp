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

/* High level structure 
TODO synths pass these down, so a file can be completely portable. Any conflicts of components with factory should be in favour of the patch...
*/
struct PatchDescriptor {
    ConnectionGraphDescriptor graph;
    std::vector<ComponentDescriptor> components;
};

}
