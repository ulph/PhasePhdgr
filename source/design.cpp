#include "design.hpp"
#include "connectiongraph.hpp"
#include "components.hpp"
#include <set>
#include <list>
#include <queue>

using namespace std;

namespace PhasePhckr {

typedef pair<string, string> ModulePortPair;
typedef map<ModulePortPair, list<ModulePortPair>> AliasMap;

void designConnectionGraph(
    ConnectionGraph &g,
    ConnectionGraphDescriptor& gDesc,
    map<string, int>& handles,
    const ComponentRegister & cp,
    AliasMap &componentInputMappings,
    AliasMap &componentOutputMappings
);

bool unpackComponent(
    ConnectionGraph &g,
    const ModuleVariable &mv, 
    map<string, int>&handles,
    const ComponentRegister & cp,
    AliasMap &componentInputMappings,
    AliasMap &componentOutputMappings
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
        const auto k = make_pair(mv.name, i.alias);
        componentInputMappings[k] = list<ModulePortPair>();
        for (auto& w : i.wrapped) {
            w.module = pfx + w.module;
            componentInputMappings[k].push_back(make_pair(w.module, w.port));
        }
    }
    for (auto &o : cD.outputs) {
        const auto k = make_pair(mv.name, o.alias);
        componentOutputMappings[k] = list<ModulePortPair>();
        for (auto& w : o.wrapped) {
            w.module = pfx + w.module;
            componentOutputMappings[k].push_back(make_pair(w.module, w.port));
        }
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
    AliasMap &componentInputMappings,
    bool input
) {

    // deal with the remapped connections and values
    // TODO, this must be possible some more elegant way ...

    // first, find all the virtual ports that map to some place else
    list<ModulePortPair> componentPorts;
    for (const auto &c : components) {
        for (const auto& s : componentInputMappings) {
            if (s.first.first == c) {
                componentPorts.push_back(s.first);
            }
        }
    }

    // then, track down to where they branch out
    while (componentPorts.size()) {
        auto from = componentPorts.back(); componentPorts.pop_back();
        auto it = componentInputMappings.find(from);
        if (it == componentInputMappings.end()) continue;
        queue<ModulePortPair> traversalQueue;
        for (auto &to : it->second) {
            traversalQueue.emplace(to);
        }
        while (traversalQueue.size()) {
            const auto n = traversalQueue.front(); traversalQueue.pop();
            auto nit = componentInputMappings.find(n);
            if (nit == componentInputMappings.end()) {
                // we are done, do the remap using n
                cout << (input ? "remap input: " : "remap output: ") << from.first << ":" << from.second << " => " << n.first << ":" << n.second;
                for (auto &c : gDesc.connections) {
                    if (input && c.target.module == from.first && c.target.port == from.second) {
                        c.target.module = n.first;
                        c.target.port = n.second;
                        cout << " p";
                    }
                    else if (!input && c.source.module == from.first && c.source.port == from.second) {
                        c.source.module = n.first;
                        c.source.port = n.second;
                        cout << " p";
                    }
                }
                if (input) {
                    for (auto &v : gDesc.values) {
                        if (v.target.module == from.first && v.target.port == from.second) {
                            v.target.module = n.first;
                            v.target.port = n.second;
                            cout << " v";
                        }
                    }
                }
                cout << endl;
                continue;
            };
            // not done, continue branching out (potentially)
            for (auto &to : nit->second) {
                traversalQueue.emplace(to);
            }
        }
    }
}


void designConnectionGraph(
    ConnectionGraph &g,
    ConnectionGraphDescriptor &gDesc,
    map<string, int> &handles,
    const ComponentRegister &cp,
    AliasMap &componentInputMappings,
    AliasMap &componentOutputMappings
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
    AliasMap componentInputMappings;
    AliasMap componentOutputMappings;
    ComponentRegister cp = cpGlobal;
    for (const auto & c : description.components) {
        cp.registerComponent(c.first, c.second);
    }
    designConnectionGraph(
        connectionGraph, 
        description.root, 
        moduleHandles, 
        cp, 
        componentInputMappings, 
        componentOutputMappings
    );
}

}