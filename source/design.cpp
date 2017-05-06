#include "design.hpp"
#include "connectiongraph.hpp"
#include "components.hpp"
#include <set>
#include <list>

namespace PhasePhckr {

void designConnectionGraph(
    ConnectionGraph &g,
    ConnectionGraphDescriptor& gDesc,
    map<string, int>& handles,
    const ComponentRegister & cp,
    map<pair<string, string>, pair<string, string>> &componentInputMappings,
    map<pair<string, string>, pair<string, string>> &componentOutputMappings
);

bool unpackComponent(
    ConnectionGraph &g,
    const ModuleVariable &mv, 
    map<string, int>&handles,
    const ComponentRegister & cp,
    map<pair<string, string>, pair<string, string>> &componentInputMappings,
    map<pair<string, string>, pair<string, string>> &componentOutputMappings
) {
    ComponentDescriptor cD;

    // find the component definition
    cout << "component " << mv.name << " " << mv.type << endl;
    if (!cp.getComponent(mv.type, cD)) {
        cerr << "Error: " << mv.name << " " << mv.type << " component unknown! (modules)" << endl;
        return false;
    }

    // move component subgraph onto the mv's "scope"
    const string pfx = mv.name + ".";
    for (auto &i : cD.inputs) {
        componentInputMappings[make_pair( mv.name, i.alias )] = make_pair( pfx + i.wrapped.module, i.wrapped.port );
        i.wrapped.module = pfx + i.wrapped.module;
    }
    for (auto &o : cD.outputs) {
        componentOutputMappings[make_pair( mv.name, o.alias )] = make_pair( pfx + o.wrapped.module, o.wrapped.port );
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

    // parse the sub graph
    designConnectionGraph(g, cD.graph, handles, cp, componentInputMappings, componentOutputMappings);

    return true;
}


void mapComponentPorts(
    ConnectionGraphDescriptor &gDesc,
    set<string> components,
    map<pair<string, string>, pair<string, string>> &componentInputMappings,
    bool input
) {

    // deal with the remapped connections and values
    list<pair<string, string>> componentPorts;
    for (const auto &c : components) {
        for (const auto& s : componentInputMappings) {
            if (s.first.first == c) {
                componentPorts.push_back(s.first);
            }
        }
    }
    while (componentPorts.size()) {
        auto from = componentPorts.back(); componentPorts.pop_back();
        auto to = from;
        auto it = componentInputMappings.find(to);
        while (it != componentInputMappings.end()) {
            to = it->second;
            it = componentInputMappings.find(to);
        }
        cout << (input ? "remap input: " : "remap output: ") << from.first << ":" << from.second << " => " << to.first << ":" << to.second;
        for (auto &c : gDesc.connections) {
            if (input && c.target.module == from.first && c.target.port == from.second) {
                c.target.module = to.first;
                c.target.port = to.second;
                cout << " p";
            }
            else if (!input && c.source.module == from.first && c.source.port == from.second) {
                c.source.module = to.first;
                c.source.port = to.second;
                cout << " p";
            }
        }
        if (input) {
            for (auto &v : gDesc.values) {
                if (v.target.module == from.first && v.target.port == from.second) {
                    v.target.module = to.first;
                    v.target.port = to.second;
                    cout << " v";
                }
            }
        }
        cout << endl;
    }

}


void designConnectionGraph(
    ConnectionGraph &g,
    ConnectionGraphDescriptor &gDesc,
    map<string, int> &handles,
    const ComponentRegister &cp,
    map<pair<string, string>, pair<string, string>> &componentInputMappings,
    map<pair<string, string>, pair<string, string>> &componentOutputMappings
) {
    set<string> components;

    // find any components (module bundles) and unpack them
    for (const auto &m : gDesc.modules) {
        if (m.type.front() == '@') {
            if (unpackComponent(g, m, handles, cp, componentInputMappings, componentOutputMappings)) {
                components.insert(m.name);
            }
        }
    }

    mapComponentPorts(gDesc, components, componentInputMappings, true);
    mapComponentPorts(gDesc, components, componentOutputMappings, false);

    // create the modules and store their handles
    for (const auto &m : gDesc.modules) {
        if (components.count(m.name)) continue;
        if (handles.count(m.name) > 0) {
            cerr << "Error: " << m.name << " name dupe! (modules)" << endl;
        }
        else {
            int handle = g.addModule(m.type);
            handles[m.name] = handle;
            cout << handle << " : " << " " << m.name << std::endl;
        }
    }

    // iterate over the connections
    for (const auto &c : gDesc.connections) {
        int hFrom = handles.count(c.source.module) > 0 ? handles.at(c.source.module) : -2;
        int hTo = handles.count(c.target.module) > 0 ? handles.at(c.target.module) : -2;
        cout << c.source.module << ":" << c.source.port << " -> " << c.target.module << ":" << c.target.port << endl;
        if (hFrom < 0 || hTo < 0) {
            cerr << "Error: \"" << c.source.module << " -> " << c.target.module << "\" invalid. ";
            if (hFrom < 0) {
                cerr << " \"" << c.source.module << "\" not found! ";
            }
            if (hTo < 0) {
                cerr << " \"" << c.target.module << "\" not found! ";
            }
            cerr << endl;
        }
        else {
            g.connect(hFrom, c.source.port, hTo, c.target.port);
        }
    }

    // set default values provided
    for (const auto &v : gDesc.values) {
        if (components.count(v.target.module)) continue;
        int h = handles.count(v.target.module) > 0 ? handles.at(v.target.module) : -2;
        cout << v.target.module << ":" << v.target.port << " = " << v.value << endl;
        if (h < 0) {
            cerr << "Error: " << v.target.module << " not found! (values)" << endl;
        }
        else {
            g.setInput(h, v.target.port, v.value);
        }
    }
}

void designConnectionGraph(
    ConnectionGraph &connectionGraph,
    PatchDescriptor &description,
    map<string, int> &moduleHandles,
    const ComponentRegister & cpGlobal
) {
    map<pair<string, string>, pair<string, string>> componentInputMappings;
    map<pair<string, string>, pair<string, string>> componentOutputMappings;
    ComponentRegister cp = cpGlobal;
    for (const auto & c : description.components) {
        cp.registerComponent(c.first, c.second);
    }
    designConnectionGraph(connectionGraph, description.root, moduleHandles, cp, componentInputMappings, componentOutputMappings);
}

}