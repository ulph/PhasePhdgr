#include "design.hpp"
#include "connectiongraph.hpp"
#include "components.hpp"

namespace PhasePhckr {

bool unpackComponent(
    ConnectionGraph &connectionGraph,
    const ModuleVariable &mv, 
    std::map<std::string, int>&mh,
    ComponentDescriptor &comp
) {
    std::cout << "component " << mv.name << " " << mv.type << std::endl;
    if (!getComponent(mv.type, comp)) {
        std::cerr << "Error: " << mv.name << " " << mv.type << " component unknown! (modules)" << std::endl;
        return false;
    }
    const std::string pfx = mv.name + ".";
    for (auto &i : comp.inputs) {
        i.target.module = pfx + i.target.module;
    }
    for (auto &o : comp.outputs) {
        o.target.module = pfx + o.target.module;
    }
    for (auto &m : comp.graph.modules) {
        m.name = pfx + m.name;
    }
    for (auto &v : comp.graph.values) {
        v.target.module = pfx + v.target.module;
    }
    for (auto &c : comp.graph.connections) {
        c.source.module = pfx + c.source.module;
        c.target.module = pfx + c.target.module;
    }
    // parse the sub graph
    DesignConnectionGraph(connectionGraph, comp.graph, mh);
    return true;
}

void DesignConnectionGraph(
    ConnectionGraph &connectionGraph,
    ConnectionGraphDescriptor& p,
    std::map<std::string, int>&moduleHandles
) {
    std::map<std::string, ComponentDescriptor> componentDescriptors;

    // find any components (module bundles) and unpack them
    for (const auto &m : p.modules) {
        if (m.type.front() == '@') {
            ComponentDescriptor comp;
            if (unpackComponent(connectionGraph, m, moduleHandles, comp)) {
                componentDescriptors[m.name] = comp;
            }
        }
    }

    if (componentDescriptors.size()) {
        // modify any connections involving components
        for (auto &c : p.connections) {
            // from a component
            if (componentDescriptors.count(c.source.module)){
                auto cmp = componentDescriptors[c.source.module];
                for (auto ao : cmp.outputs) {
                    if (ao.alias == c.source.port) {
                        c.source.module = ao.target.module;
                        c.source.port = ao.target.port;
                    }
                }
            }
            // to a component
            if (componentDescriptors.count(c.target.module)) {
                auto cmp = componentDescriptors[c.target.module];
                for (auto ai : cmp.inputs) {
                    if (ai.alias == c.target.port) {
                        c.target.module = ai.target.module;
                        c.target.port = ai.target.port;
                    }
                }
            }
        }

        // modify any values involving components
        for (auto &v : p.values) {
            if (componentDescriptors.count(v.target.module)) {
                auto cmp = componentDescriptors[v.target.module];
                for (auto ai : cmp.inputs) {
                    if (ai.alias == v.target.port) {
                        v.target.module = ai.target.module;
                        v.target.port = ai.target.port;
                    }
                }
            }
        }
    }

    // create the modules and store their handles
    for (const auto &m : p.modules) {
        if (componentDescriptors.count(m.name)) continue; // skip any components
        if (moduleHandles.count(m.name) > 0) {
            std::cerr << "Error: " << m.name << " name dupe! (modules)" << std::endl;
        }
        else {
            int handle = connectionGraph.addModule(m.type);
            moduleHandles[m.name] = handle;
            std::cout << handle << " : " << " " << m.name << std::endl;
        }
    }

    // iterate over the connections
    for (const auto &c : p.connections) {
        int fromModuleHandle = moduleHandles.count(c.source.module) > 0 ? moduleHandles.at(c.source.module) : -2;
        int toModuleHandle = moduleHandles.count(c.target.module) > 0 ? moduleHandles.at(c.target.module) : -2;
        if (fromModuleHandle < 0) {
            std::cerr << "Error: " << c.source.module << " not found! (connection source)" << std::endl;
        }
        else if (toModuleHandle < 0) {
            std::cerr << "Error: " << c.target.module << " not found! (connection target)" << std::endl;
        }
        else {
            connectionGraph.connect(fromModuleHandle, c.source.port, toModuleHandle, c.target.port);
            std::cout << c.source.module << ":" << c.source.port << " -> " << c.target.module << ":" << c.target.port << std::endl;
        }
    }

    // set default values provided
    for (const auto &v : p.values) {
        if (componentDescriptors.count(v.target.module)) continue;
        int targetHandle = moduleHandles.count(v.target.module) > 0 ? moduleHandles.at(v.target.module) : -2;
        if (targetHandle < 0) {
            std::cerr << "Error: " << v.target.module << " not found! (values)" << std::endl;
        }
        else {
            connectionGraph.setInput(targetHandle, v.target.port, v.value);
            std::cout << v.target.module << ":" << v.target.port << " = " << v.value << std::endl;
        }
    }
}

}