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

void to_json(json& j, const ModulePosition& xy) {
    j[0] = xy.x;
    j[1] = xy.y;
}

void from_json(const json& j, ModulePosition& xy) {
    xy.x = j.at(0).get<int>();
    xy.y = j.at(1).get<int>();
}

void to_json(json& j, const PatchDescriptor& p) {
    j["modules"] = p.root.modules;
    j["connections"] = p.root.connections;
    j["values"] = p.root.values;
    j["docString"] = p.docString;
    j["components"] = p.components;
    j["layout"] = p.layout;
}

void from_json(const json& j, PatchDescriptor& p) {
    p.root.modules = j.at("modules").get<vector<ModuleVariable>>();
    p.root.connections = j.at("connections").get<vector<ModulePortConnection>>();
    p.root.values = j.at("values").get<vector<ModulePortValue>>();
    p.docString = j.at("docString").get<string>();
    p.components = j.at("components").get<map<string, ComponentDescriptor>>();
    p.layout = j.at("layout").get<map<string, ModulePosition>>();
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
