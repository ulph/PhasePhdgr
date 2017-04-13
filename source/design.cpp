#include "design.hpp"

namespace PhasePhckr {

std::map<std::string, int> DesignConnectionGraph(
    ConnectionGraph &connectionGraph,
    const ConnectionGraphDescriptor &p
) {
    // internal place to bounce strings to handles
    std::map<std::string, int> moduleHandles;

    // create the modules and store their handles
    for (const auto &m : p.modules) {
        if (moduleHandles.count(m.name) > 0) {
            std::cerr << "Error: " << m.name << " name dupe! (modules)" << std::endl;
        }
        else {
            moduleHandles[m.name] = connectionGraph.addModule(m.type);
        }
    }

    // iterate over the connections
    for (const auto &c : p.connections) {
        int fromModuleHandle = moduleHandles.count(c.from.module) > 0 ? moduleHandles.at(c.from.module) : -2;
        int toModuleHandle = moduleHandles.count(c.to.module) > 0 ? moduleHandles.at(c.to.module) : -2;
        if (fromModuleHandle < 0) {
            std::cerr << "Error: " << c.from.module << " not found! (connections)" << std::endl;
        }
        else if (toModuleHandle < 0) {
            std::cerr << "Error: " << c.to.module << " not found! (connections)" << std::endl;
        }
        else {
            connectionGraph.connect(fromModuleHandle, c.from.port, toModuleHandle, c.to.port);
        }
    }

    // set default values provided
    for (const auto &v : p.values) {
        int targetHandle = moduleHandles.count(v.target.module) > 0 ? moduleHandles.at(v.target.module) : -2;
        if (targetHandle < 0) {
            std::cerr << "Error: " << v.target.module << "not found! (values)" << std::endl;
        }
        else {
            connectionGraph.setInput(targetHandle, v.target.port, v.value);
        }
    }

    return moduleHandles;
}

}