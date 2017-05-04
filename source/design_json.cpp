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

void to_json(json& j, const ModulePortAlias& cgd) {
    j["alias"] = cgd.alias;
    j["wrapped"] = cgd.wrapped;
}

void from_json(const json& j, ModulePortAlias& cgd) {
    cgd.alias = j.at("alias").get<std::string>();
    cgd.wrapped = j.at("wrapped").get<ModulePort>();
}

void to_json(json& j, const ComponentDescriptor& cgd) {
    j["inputs"] = cgd.inputs;
    j["outputs"] = cgd.outputs;
    j["graph"] = cgd.graph;
    j["docString"] = cgd.docString;
}

void from_json(const json& j, ComponentDescriptor& cgd) {
    cgd.inputs = j.at("inputs").get<std::vector<ModulePortAlias>>();
    cgd.outputs = j.at("outputs").get<std::vector<ModulePortAlias>>();
    cgd.graph = j.at("graph").get<ConnectionGraphDescriptor>();
    cgd.docString = j.at("docString").get<std::string>();
}

}
