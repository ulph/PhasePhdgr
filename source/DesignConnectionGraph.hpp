#pragma once

#include <utility>
#include <vector>
#include <map>

#include "connectiongraph.hpp"

namespace PhasePhckr {

struct ModuleVariable {
    const char * name;
    const char * type;
};

struct ModulePortConnection {
    const char * fromModule;
    const char * fromPort;
    const char * toModule;
    const char * toPort;
};

struct ModulePortValue {
    const char * module;
    const char * port;
    float value;
};

struct BusHandles {
    int inBus;
    int outBus;
    BusHandles(int inBus, int outBus) : inBus(inBus), outBus(outBus){}
};

BusHandles DesignConnectionGraph(
    ConnectionGraph & connectionGraph,
    ModuleVariable inputBusVariable,
    ModuleVariable outputBusVariable,
    const std::vector<ModuleVariable> &modules,
    const std::vector<ModulePortConnection> &connections,
    const std::vector<ModulePortValue> defaultValues
){
    // internal place to bounce strings to handles
    std::map<const char*, int> moduleHandles;

    // output - outside wants the handles for these two special modules
    BusHandles bus(
        connectionGraph.addModule(inputBusVariable.type),
        connectionGraph.addModule(outputBusVariable.type)
    );
    moduleHandles[inputBusVariable.name] = bus.inBus;
    moduleHandles[outputBusVariable.name] = bus.outBus;

    // create the modules and store their handles
    for(const auto &m:modules){
        if(moduleHandles.count(m.name) > 0){
            // some error printout
        }
        else{
            moduleHandles[m.name] = connectionGraph.addModule(m.type);
        }
    }

    // iterate over the connections
    for(const auto &c:connections){
        int fromModuleHandle = moduleHandles.count(c.fromModule) > 0 ? moduleHandles.at(c.fromModule) : -2;
        int toModuleHandle = moduleHandles.count(c.toModule) > 0 ? moduleHandles.at(c.toModule) : -2;
        if( fromModuleHandle >= 0 && toModuleHandle >= 0 ){
            connectionGraph.connect(fromModuleHandle, c.fromPort, toModuleHandle, c.toPort);
        }
        else {
            // some error printout
        }
    }

    // set default values provided
    for(const auto &v:defaultValues){
        int targetHandle = moduleHandles.count(v.module) > 0 ? moduleHandles.at(v.module) : -2;
        if( targetHandle >= 0) {
            connectionGraph.setInput(targetHandle, v.port, v.value);
        }
        else {
            // some error printout
        }
    }

    return bus;
}

}
