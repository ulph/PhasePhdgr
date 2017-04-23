#include "design_json.hpp"

#include <sstream>

namespace PhasePhckr{

void to_json(json& j, const ModuleVariable& mv) {
    j[0] = mv.name;
    j[1] = mv.type;
}

void from_json(const json& j, ModuleVariable& mv) {
    mv.name = j.at(0).get<std::string>();
    mv.type = j.at(1).get<std::string>();
}

void to_json(json& j, const ModulePort& mp) {
    j[0] = mp.module;
    j[1] = mp.port;
}

void from_json(const json& j, ModulePort& mp) {
    mp.module = j.at(0).get<std::string>();
    mp.port = j.at(1).get<std::string>();
}

void to_json(json& j, const ModulePortValue& mpv) {
    j[0] = mpv.target.module;
    j[1] = mpv.target.port;
    j[2] = mpv.value;
}

void from_json(const json& j, ModulePortValue& mpv) {
    mpv.target.module = j.at(0).get<std::string>();
    mpv.target.port = j.at(1).get<std::string>();
    mpv.value = j.at(2);
}

void to_json(json& j, const ModulePortConnection& mpc) {
    j[0] = mpc.source.module;
    j[1] = mpc.source.port;
    j[2] = mpc.target.module;
    j[3] = mpc.target.port;
}

void from_json(const json& j, ModulePortConnection& mpc) {
    mpc.source.module = j.at(0).get<std::string>();
    mpc.source.port = j.at(1).get<std::string>();
    mpc.target.module = j.at(2).get<std::string>();
    mpc.target.port = j.at(3).get<std::string>();
}

void to_json(json& j, const ModulePortConnections& mpcs) {
    j[0] = mpcs.source;
    j[1] = mpcs.targets;
}

void from_json(const json& j, ModulePortConnections& mpcs) {
    mpcs.source = j.at(0).get<ModulePort>();
    mpcs.targets = j.at(1).get<std::vector<ModulePort>>();
}

void to_json(json& j, const ConnectionGraphDescriptor& cgd) {
    j["modules"] = cgd.modules;
    j["connections"] = cgd.connections;
    j["values"] = cgd.values;
}

void from_json(const json& j, ConnectionGraphDescriptor& cgd) {
    cgd.modules = j.at("modules").get<std::vector<ModuleVariable>>();
    cgd.connections = j.at("connections").get<std::vector<ModulePortConnection>>();
    cgd.values = j.at("values").get<std::vector<ModulePortValue>>();
}

void to_json(json& j, const ModulePortAlias& mv) {
    j[0] = mv.alias;
    j[1] = mv.target;
}

void from_json(const json& j, ModulePortAlias& mv) {
    mv.alias = j.at(0).get<std::string>();
    mv.target = j.at(1).get<ModulePort>();
}

void to_json(json& j, const ComponentDescriptor& cgd) {
    j["modules"] = cgd.graph.modules;
    j["connections"] = cgd.graph.connections;
    j["values"] = cgd.graph.values;
    j["inputs"] = cgd.inputs;
    j["outputs"] = cgd.outputs;
}

void from_json(const json& j, ComponentDescriptor& cgd) {
    cgd.graph.modules = j.at("modules").get<std::vector<ModuleVariable>>();
    cgd.graph.connections = j.at("connections").get<std::vector<ModulePortConnection>>();
    cgd.graph.values = j.at("values").get<std::vector<ModulePortValue>>();
    cgd.inputs = j.at("inputs").get<std::vector<ModulePortAlias>>();
    cgd.outputs = j.at("outputs").get<std::vector<ModulePortAlias>>();
}

void to_json(json& j, const PatchDescriptor& cgd) {
    j["voice"] = cgd.voiceGraph;
    j["effect"] = cgd.effectGraph;
}

void from_json(const json& j, PatchDescriptor& cgd) {
    cgd.voiceGraph = j.at("voice");
    cgd.effectGraph = j.at("effect");
}

}
