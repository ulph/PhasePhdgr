#include "design_json.hpp"

#include <sstream>

namespace PhasePhckr{

void to_json(json& j, const ModuleVariable& mv) {
    j[0] = mv.name;
    j[1] = mv.type;
}

void from_json(const json& j, ModuleVariable& mv) {
    mv.name = j.at(0).get<string>();
    mv.type = j.at(1).get<string>();
}

void to_json(json& j, const ModulePort& mp) {
    j[0] = mp.module;
    j[1] = mp.port;
}

void from_json(const json& j, ModulePort& mp) {
    mp.module = j.at(0).get<string>();
    mp.port = j.at(1).get<string>();
}

void to_json(json& j, const ModulePortValue& mpv) {
    j[0] = mpv.target.module;
    j[1] = mpv.target.port;
    j[2] = mpv.value;
}

void from_json(const json& j, ModulePortValue& mpv) {
    mpv.target.module = j.at(0).get<string>();
    mpv.target.port = j.at(1).get<string>();
    mpv.value = j.at(2);
}

void to_json(json& j, const ModulePortConnection& mpc) {
    j[0] = mpc.source.module;
    j[1] = mpc.source.port;
    j[2] = mpc.target.module;
    j[3] = mpc.target.port;
}

void from_json(const json& j, ModulePortConnection& mpc) {
    mpc.source.module = j.at(0).get<string>();
    mpc.source.port = j.at(1).get<string>();
    mpc.target.module = j.at(2).get<string>();
    mpc.target.port = j.at(3).get<string>();
}

void to_json(json& j, const ConnectionGraphDescriptor& cgd) {
    j["modules"] = cgd.modules;
    j["connections"] = cgd.connections;
    j["values"] = cgd.values;
}

void from_json(const json& j, ConnectionGraphDescriptor& cgd) {
    cgd.modules = j.at("modules").get<vector<ModuleVariable>>();
    cgd.connections = j.at("connections").get<vector<ModulePortConnection>>();
    cgd.values = j.at("values").get<vector<ModulePortValue>>();
}

void to_json(json& j, const PatchDescriptor& cgd) {
    j["modules"] = cgd.root.modules;
    j["connections"] = cgd.root.connections;
    j["values"] = cgd.root.values;
    j["docString"] = cgd.docString;
    j["components"] = cgd.components;
}

void from_json(const json& j, PatchDescriptor& cgd) {
    cgd.root.modules = j.at("modules").get<vector<ModuleVariable>>();
    cgd.root.connections = j.at("connections").get<vector<ModulePortConnection>>();
    cgd.root.values = j.at("values").get<vector<ModulePortValue>>();
    cgd.docString = j.at("docString").get<string>();
    cgd.components = j.at("components").get<map<string, ComponentDescriptor>>();
}

void to_json(json& j, const ModulePortAlias& cgd) {
    j["alias"] = cgd.alias;
    j["wrapped"] = cgd.wrapped;
}

void from_json(const json& j, ModulePortAlias& cgd) {
    cgd.alias = j.at("alias").get<string>();
    cgd.wrapped = j.at("wrapped").get<vector<ModulePort>>();
}

void to_json(json& j, const ComponentDescriptor& cgd) {
    j["inputs"] = cgd.inputs;
    j["outputs"] = cgd.outputs;
    j["graph"] = cgd.graph;
    j["docString"] = cgd.docString;
}

void from_json(const json& j, ComponentDescriptor& cgd) {
    cgd.inputs = j.at("inputs").get<vector<ModulePortAlias>>();
    cgd.outputs = j.at("outputs").get<vector<ModulePortAlias>>();
    cgd.graph = j.at("graph").get<ConnectionGraphDescriptor>();
    cgd.docString = j.at("docString").get<string>();
}

}
