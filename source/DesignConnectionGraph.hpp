#pragma once

#include <utility>
#include <vector>
#include <map>

#include "connectiongraph.hpp"

#if USING_NLOHMANN_JSON
#include "nlohmann/json.hpp"
using nlohmann::json;
#endif // USING_NLOHMANN_JSON

namespace PhasePhckr {

struct ModuleVariable {
    std::string name;
    std::string type;
};

struct ModulePort {
    std::string name;
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

struct BusHandles {
    int inBus;
    int outBus;
    BusHandles(int inBus, int outBus) : inBus(inBus), outBus(outBus){}
};

struct Patch {
    ModuleVariable input;
    ModuleVariable output;
    std::vector<ModuleVariable> modules;
    std::vector<ModulePortConnection> connections;
    std::vector<ModulePortValue> values;
};

#if USING_NLOHMANN_JSON

void to_json(json& j, const ModuleVariable& m) {
    j = json{
        {"name", m.name}, 
        {"type", m.type}
    };
}

void from_json(const json& j, ModuleVariable& m) {
    m = ModuleVariable();
    m.name = j["name"];
    m.type = j["type"];
}

void to_json(json& j, const ModulePort& m) {
    j = json{
        { "name", m.name },
        { "port", m.port }
    };
}

void from_json(const json& j, ModulePort& m) {
    m = ModulePort();
    m.name = j["name"];
    m.port = j["port"];
}

void to_json(json& j, const ModulePortConnection& m) {
    j = json{ 
        { "to", m.to }, 
        { "from", m.from }
    };
}

void from_json(const json& j, ModulePortConnection& m) {
    m = ModulePortConnection();
    m.to = j["to"];
    m.from = j["from"];
}

void to_json(json& j, const ModulePortValue& m) {
    j = json{
        { "target", m.target },
        { "value", m.value },
    };
}

void from_json(const json& j, ModulePortValue& m) {
    m = ModulePortValue();
    m.target = j["target"];
    m.value = j["value"];
}

void to_json(json& j, const Patch& m) {
    j = json{
        { "input", m.input },
        { "output", m.output },
        { "modules", m.modules },
        { "connections", m.connections },
        { "values", m.values },
    };
}

void from_json(const json& j, Patch& m) {
    m = Patch();
    m.input = j["input"];
    m.output = j["output"];
    m.modules = j["modules"].get<std::vector<ModuleVariable>>();
    m.connections = j["connections"].get<std::vector<ModulePortConnection>>();
    m.values = j["values"].get<std::vector<ModulePortValue>>();
}

BusHandles DesignConnectionGraph(
    ConnectionGraph &connectionGraph,
    const json &patch
) {
    return DesignConnectionGraph(connectionGraph, patch.get<Patch>());
}

#endif // USING_NLOHMANN_JSON

BusHandles DesignConnectionGraph(
    ConnectionGraph &connectionGraph,
    const Patch &patch
){
    const Patch &p = patch;

    // internal place to bounce strings to handles
    std::map<std::string, int> moduleHandles;

    // output - outside wants the handles for these two special modules
    BusHandles bus(
        connectionGraph.addModule(p.input.type),
        connectionGraph.addModule(p.output.type)
    );
    moduleHandles[p.input.name] = bus.inBus;
    moduleHandles[p.output.name] = bus.outBus;

    // create the modules and store their handles
    for(const auto &m:p.modules){
        if(moduleHandles.count(m.name) > 0){
            // some error printout
        }
        else{
            moduleHandles[m.name] = connectionGraph.addModule(m.type);
        }
    }

    // iterate over the connections
    for(const auto &c:p.connections){
        int fromModuleHandle = moduleHandles.count(c.from.name) > 0 ? moduleHandles.at(c.from.name) : -2;
        int toModuleHandle = moduleHandles.count(c.to.name) > 0 ? moduleHandles.at(c.to.name) : -2;
        if( fromModuleHandle >= 0 && toModuleHandle >= 0 ){
            connectionGraph.connect(fromModuleHandle, c.from.port, toModuleHandle, c.to.port);
        }
        else {
            // some error printout
        }
    }

    // set default values provided
    for(const auto &v:p.values){
        int targetHandle = moduleHandles.count(v.target.name) > 0 ? moduleHandles.at(v.target.name) : -2;
        if( targetHandle >= 0) {
            connectionGraph.setInput(targetHandle, v.target.port, v.value);
        }
        else {
            // some error printout
        }
    }

    return bus;
}

}
