#include <phasephckr.hpp>
#include <phasephckr_json.hpp>

#include <sstream>

namespace PhasePhckr{

void to_json(json& j, const PadDescription& p) {
    j[0] = p.name;
    j[1] = p.unit;
    j[2] = p.defaultValue;
}

void from_json(const json& j, PadDescription& p) {
    p.name = j.at(0).get<string>();
    p.unit = j.at(1).get<string>();
    p.defaultValue = j.at(2).get<float>();
}

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
    vector<ModuleVariable> modules;
    for (const auto& kv : cgd.modules) {
        modules.emplace_back(ModuleVariable(kv.first, kv.second));
    }
    j["modules"] = modules;
    j["connections"] = cgd.connections;
    vector<ModulePortValue> values;
    for (const auto& kv : cgd.values) {
        values.emplace_back(ModulePortValue(kv.first, kv.second));
    }
    j["values"] = values;
}

void from_json(const json& j, ConnectionGraphDescriptor& cgd) {
    auto modules = j.at("modules").get<vector<ModuleVariable>>();
    cgd.modules.clear();
    for (const auto& m : modules) {
        cgd.modules[m.name] = m.type;
    }
    cgd.connections = j.at("connections").get<vector<ModulePortConnection>>();
    auto values = j.at("values").get<vector<ModulePortValue>>();
    for (const auto& mpv : values) {
        cgd.values[mpv.target] = mpv.value;
    }
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
    j["parameters"] = p.parameters;
}

void from_json(const json& j, PatchDescriptor& p) {
    p.root.graph = j.at("graph").get<ConnectionGraphDescriptor>();
    p.root.docString = j.at("docString").get<string>();
    p.components = j.at("components").get<map<string, ComponentDescriptor>>();
    p.layout = j.at("layout").get<map<string, ModulePosition>>();
    if (j.count("parameters")) p.parameters = j.at("parameters").get<vector<PatchParameterDescriptor>>();
}

void to_json(json& j, const ComponentDescriptor& cgd) {
    j[c_inBus.name] = cgd.inBus;
    j[c_outBus.name] = cgd.outBus;
    j["graph"] = cgd.graph;
    j["docString"] = cgd.docString;
}

void from_json(const json& j, ComponentDescriptor& cgd) {
    cgd.inBus = j.at(c_inBus.name).get<vector<PadDescription>>();
    cgd.outBus = j.at(c_outBus.name).get<vector<PadDescription>>();
    cgd.graph = j.at("graph").get<ConnectionGraphDescriptor>();
    cgd.docString = j.at("docString").get<string>();
}

void to_json(json& j, const PatchParameterDescriptor& p) {
    j["id"] = p.id;
    j["type"] = p.type;
    j["value"] = p.val;
    j["min"] = p.min;
    j["max"] = p.max;
}

void from_json(const json& j, PatchParameterDescriptor& p) {
    p.id = j.at("id").get<string>();
    if (j.count("type")) {
        p.type = j.at("type");
    }
    else {
        p.type = UNDEFINED;
        if (p.id.substr(0, 2) == "e ") {
            p.type = EFFECT;
            p.id = p.id.substr(2);
        }
        else if (p.id.substr(0, 2) == "v ") {
            p.type = VOICE;
            p.id = p.id.substr(2);
        }
    }
    p.val = j.at("value").get<float>();
    p.min = j.value("min", 0.f);
    p.max = j.value("max", 1.f);
}

void to_json(json& j, const PresetParameterDescriptor& p) {
    j["index"] = p.index;
    j["id"] = p.p.id;
    j["type"] = p.p.type;
    j["value"] = p.p.val;
    j["min"] = p.p.min;
    j["max"] = p.p.max;
}

void from_json(const json& j, PresetParameterDescriptor& p) {
    p.index = j.at("index").get<int>();
    p.p = j.get<PatchParameterDescriptor>();
}

void to_json(json& j, const PresetDescriptor& preset) {
    j["voice"] = preset.voice;
    j["effect"] = preset.effect;
    j["parameters"] = preset.parameters;
}

void from_json(const json& j, PresetDescriptor& preset) {
    preset.voice = j.at("voice").get<PatchDescriptor>();
    preset.effect = j.at("effect").get<PatchDescriptor>();
    if(j.count("parameters")) preset.parameters = j.at("parameters").get<vector<PresetParameterDescriptor>>();
}

}
