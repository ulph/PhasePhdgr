#include "design.hpp"
#include "connectiongraph.hpp"
#include "components.hpp"
#include <set>

namespace PhasePhckr {

void applyComponent(
    ConnectionGraphDescriptor& gDesc,
    ComponentDescriptor &cD,
    const std::string &name
) {
    // modify any connections involving components
    for (auto &c : gDesc.connections) {
        // from a component
        if (c.source.module == name) {
            for (auto ao : cD.outputs) {
                if (ao.alias == c.source.port) {
                    c.source.module = ao.wrapped.module;
                    c.source.port = ao.wrapped.port;
                }
            }
        }
        // to a component
        if (c.target.module == name) {
            for (auto ai : cD.inputs) {
                if (ai.alias == c.target.port) {
                    c.target.module = ai.wrapped.module;
                    c.target.port = ai.wrapped.port;
                }
            }
        }
    }

    // modify any values involving components
    for (auto &v : gDesc.values) {
        if (v.target.module == name) {
            for (auto ai : cD.inputs) {
                if (ai.alias == v.target.port) {
                    v.target.module = ai.wrapped.module;
                    v.target.port = ai.wrapped.port;
                }
            }
        }
    }
}

bool unpackComponent(
    ConnectionGraph &g,
    ConnectionGraphDescriptor& gDesc,
    const ModuleVariable &mv, 
    std::map<std::string, int>&handles
) {
    ComponentDescriptor cD;

    // find the component definition
    std::cout << "component " << mv.name << " " << mv.type << std::endl;
    if (!getComponent(mv.type, cD)) {
        std::cerr << "Error: " << mv.name << " " << mv.type << " component unknown! (modules)" << std::endl;
        return false;
    }

    // move component subgraph onto the mv's "namespace"
    const std::string pfx = mv.name + ".";
    for (auto &i : cD.inputs) {
        i.wrapped.module = pfx + i.wrapped.module;
    }
    for (auto &o : cD.outputs) {
        o.wrapped.module = pfx + o.wrapped.module;
    }
    for (auto &m : cD.graph.modules) {
        m.name = pfx + m.name;
    }
    for (auto &v : cD.graph.values) {
        v.target.module = pfx + v.target.module;
    }
    for (auto &c : cD.graph.connections) {
        c.source.module = pfx + c.source.module;
        c.target.module = pfx + c.target.module;
    }

    // refresh the ConnectionGraphDescriptor with these new composite names
    applyComponent(gDesc, cD, mv.name);

    // parse the sub graph
    designConnectionGraph(g, cD.graph, handles);
    return true;
}

void designConnectionGraph(
    ConnectionGraph &g,
    ConnectionGraphDescriptor& gDesc,
    std::map<std::string, int>& handles
) {
    std::set<std::string> skip;

    // find any components (module bundles) and unpack them
    for (const auto &m : gDesc.modules) {
        if (m.type.front() == '@') {
            if (unpackComponent(g, gDesc, m, handles)) {
                skip.insert(m.name);
            }
        }
    }

    // create the modules and store their handles
    for (const auto &m : gDesc.modules) {
        if (skip.count(m.name)) continue;
        if (handles.count(m.name) > 0) {
            std::cerr << "Error: " << m.name << " name dupe! (modules)" << std::endl;
        }
        else {
            int handle = g.addModule(m.type);
            handles[m.name] = handle;
            std::cout << handle << " : " << " " << m.name << std::endl;
        }
    }

    // iterate over the connections
    for (const auto &c : gDesc.connections) {
        int hFrom = handles.count(c.source.module) > 0 ? handles.at(c.source.module) : -2;
        int hTo = handles.count(c.target.module) > 0 ? handles.at(c.target.module) : -2;
        if (hFrom < 0) {
            std::cerr << "Error: " << c.source.module << " not found! (connection source)" << std::endl;
        }
        else if (hTo < 0) {
            std::cerr << "Error: " << c.target.module << " not found! (connection target)" << std::endl;
        }
        else {
            g.connect(hFrom, c.source.port, hTo, c.target.port);
            std::cout << c.source.module << ":" << c.source.port << " -> " << c.target.module << ":" << c.target.port << std::endl;
        }
    }

    // set default values provided
    for (const auto &v : gDesc.values) {
        if (skip.count(v.target.module)) continue;
        int h = handles.count(v.target.module) > 0 ? handles.at(v.target.module) : -2;
        if (h < 0) {
            std::cerr << "Error: " << v.target.module << " not found! (values)" << std::endl;
        }
        else {
            g.setInput(h, v.target.port, v.value);
            std::cout << v.target.module << ":" << v.target.port << " = " << v.value << std::endl;
        }
    }
}

}