#include "design_json.hpp"

#include <sstream>

void to_json(json& j, const PadDescription& p){
    j[0] = p.name;
    j[1] = p.unit;
    j[2] = p.value;
}

void from_json(const json& j, PadDescription& p){
    p.name = j.at(0).get<string>();
    p.unit = j.at(1).get<string>();
    p.value = j.at(2).get<float>();
}

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
    j["graph"] = p.root.graph;
    j["docString"] = p.root.docString;
    j["components"] = p.components;
    j["layout"] = p.layout;
}

void from_json(const json& j, PatchDescriptor& p) {
    p.root.graph = j.at("graph").get<ConnectionGraphDescriptor>();
    p.root.docString = j.at("docString").get<string>();
    p.components = j.at("components").get<map<string, ComponentDescriptor>>();
    p.layout = j.at("layout").get<map<string, ModulePosition>>();
}

void to_json(json& j, const ComponentDescriptor& cgd) {
    j["inBus"] = cgd.inBus;
    j["outBus"] = cgd.outBus;
    j["graph"] = cgd.graph;
    j["docString"] = cgd.docString;
}

void from_json(const json& j, ComponentDescriptor& cgd) {
    cgd.inBus = j.at("inBus").get<vector<PadDescription>>();
    cgd.outBus = j.at("outBus").get<vector<PadDescription>>();
    cgd.graph = j.at("graph").get<ConnectionGraphDescriptor>();
    cgd.docString = j.at("docString").get<string>();
}

void to_json(json& j, const ParameterDescriptor& param){
    j["id"] = param.id;
    j["index"] = param.index;
    j["value"] = param.value;
    j["min"] = param.min;
    j["max"] = param.max;
}

void from_json(const json& j, ParameterDescriptor& param){
    param.id = j.at("id").get<string>();
    param.index = j.at("index").get<int>();
    param.value = j.at("value").get<float>();
    param.min = j.value("min", 0.f);
    param.max = j.value("max", 1.f);
}

void to_json(json& j, const PresetDescriptor& preset){
    j["voice"] = preset.voice;
    j["effect"] = preset.effect;
    j["parameters"] = preset.parameters;
}

void from_json(const json& j, PresetDescriptor& preset){
    preset.voice = j.at("voice").get<PatchDescriptor>();
    preset.effect = j.at("effect").get<PatchDescriptor>();
    try {
        preset.parameters = j.at("parameters").get<vector<ParameterDescriptor>>();
    } catch (const std::exception& e) {
        //assert(0);
    }
}

}
