#include "design.hpp"
#include "connectiongraph.hpp"

namespace PhasePhckr {

template <typename T>
void unpackComponent(
    ConnectionGraph &connectionGraph,
    const ModuleVariable &mv, 
    std::map<std::string, int>&mh
) {
    ComponentDescriptor comp;
    if (!getComponent(mv.type, comp)) {
        std::cerr << "Error: " << mv.name << " " << mv.type << " component unknown! (modules)" << std::endl;
        return;
    }

    const std::string pfx = mv.name + "@";
    for (auto &i : comp.input) {
        i.module = pfx + i.module;
    }
    for (auto &o : comp.output) {
        o.module = pfx + o.module;
    }
    for (auto &m : comp.graph.modules) {
        m.name = pfx + m.name;
    }
    for (auto &v : comp.graph.values) {
        v.target.module = pfx + v.target.module;
    }
    for (auto &c : comp.graph.connections) {
        c.from.module = pfx + c.from.module;
        c.to.module = pfx + c.to.module;
    }

    DesignConnectionGraph(connectionGraph, comp.graph, mh);
}

template <typename T>
void DesignConnectionGraph(
    ConnectionGraph &connectionGraph,
    const T& p,
    std::map<std::string, int>&moduleHandles
) {
    // find any components (module bundles) and unpack them
    for (const auto &m : p.modules) {
        if (m.type.front() == '@') {
            unpackComponent<T>(connectionGraph, m, moduleHandles);
        }
    }

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
}

void DesignConnectionGraph(ConnectionGraph &connectionGraph, const ConnectionGraphDescriptor & p, std::map<std::string, int>&mh) {
    DesignConnectionGraph<ConnectionGraphDescriptor>(connectionGraph, p, mh);
}

void DesignConnectionGraph(ConnectionGraph &connectionGraph, const ConnectionGraphDescriptor_Numerical & p, std::map<std::string, int>&mh) {
    DesignConnectionGraph<ConnectionGraphDescriptor_Numerical>(connectionGraph, p, mh);
}

std::map<std::string, ComponentDescriptor> g_componentRegistry;

bool registerComponent(std::string name, const ComponentDescriptor & desc) {
    if (g_componentRegistry.count(name)) return false;
    g_componentRegistry[name] = desc;
    return true;
}

bool getComponent(std::string name, ComponentDescriptor & desc) {
    if (!g_componentRegistry.count(name)) return false;
    desc = g_componentRegistry[name];
    return true;
}

}