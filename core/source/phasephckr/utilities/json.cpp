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
    if (cgd.values.size()) {
        vector<ModulePortValue> values;
        for (const auto& kv : cgd.values) {
            values.emplace_back(ModulePortValue(kv.first, kv.second));
        }
        j["values"] = values;
    }
}

void from_json(const json& j, ConnectionGraphDescriptor& cgd) {
    auto modules = j.at("modules").get<vector<ModuleVariable>>();
    cgd.modules.clear();
    for (const auto& m : modules) {
        cgd.modules[m.name] = m.type;
    }
    cgd.connections = j.at("connections").get<vector<ModulePortConnection>>();
    if (j.count("values")) {
        auto values = j.at("values").get<vector<ModulePortValue>>();
        for (const auto& mpv : values) {
            cgd.values[mpv.target] = mpv.value;
        }
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
    j["root"] = p.root;
    if(p.componentBundle.getAll().size()) j["components"] = p.componentBundle.getAll();
    if(p.parameters.size()) j["parameters"] = p.parameters;
}

void from_json(const json& j, PatchDescriptor& p) {
    if (j.count("graph")) p.root.graph = j.at("graph").get<ConnectionGraphDescriptor>(); // legacy
    else p.root = j.at("root").get<ComponentDescriptor>();
    if (j.count("components")) p.componentBundle.setAll(j.at("components").get<map<string, ComponentDescriptor>>());
    if (j.count("parameters")) p.parameters = j.at("parameters").get<vector<PatchParameterDescriptor>>();
}

void to_json(json& j, const ComponentDescriptor& cgd) {
    if (cgd.inBus.size()) j[c_inBus.name] = cgd.inBus;
    if (cgd.outBus.size()) j[c_outBus.name] = cgd.outBus;
    j["graph"] = cgd.graph;
    if (cgd.docString.size()) j["docString"] = cgd.docString;
    if (cgd.layout.size()) j["layout"] = cgd.layout;
}

void from_json(const json& j, ComponentDescriptor& cgd) {
    if (j.count(c_inBus.name)) cgd.inBus = j.at(c_inBus.name).get<vector<PadDescription>>();
    if (j.count(c_outBus.name)) cgd.outBus = j.at(c_outBus.name).get<vector<PadDescription>>();
    cgd.graph = j.at("graph").get<ConnectionGraphDescriptor>();
    if (j.count("docString")) cgd.docString = j.at("docString").get<string>();
    if (j.count("layout")) cgd.layout = j.at("layout").get<map<string, ModulePosition>>();
}

void to_json(json& j, const PatchParameterDescriptor& p) {
    j["id"] = p.id;
    j["value"] = p.v.val;
    j["min"] = p.v.min;
    j["max"] = p.v.max;
}

void from_json(const json& j, PatchParameterDescriptor& p) {
    p.id = j.at("id").get<string>();
    p.v.val = j.at("value").get<float>();
    p.v.min = j.value("min", 0.f);
    p.v.max = j.value("max", 1.f);
}

void to_json(json& j, const PresetParameterDescriptor& p) {
    j["index"] = p.index;
    j["type"] = p.type;
    j["id"] = p.p.id;
    j["value"] = p.p.v.val;
    j["min"] = p.p.v.min;
    j["max"] = p.p.v.max;
}

void from_json(const json& j, PresetParameterDescriptor& p) {
    p.index = j.at("index").get<int>();
    p.p = j.get<PatchParameterDescriptor>();
    if (j.count("type")) p.type = j.at("type");
    else {
        // if type is missing, try legacy style
        p.type = UNDEFINED;
        if (p.p.id.substr(0, 2) == "e ") {
            p.type = EFFECT;
            p.p.id = p.p.id.substr(2);
        }
        else if (p.p.id.substr(0, 2) == "v ") {
            p.type = VOICE;
            p.p.id = p.p.id.substr(2);
        }
    }
}

void to_json(json& j, const PresetDescriptor& preset) {
    j["voice"] = preset.voice;
    j["effect"] = preset.effect;
    if(preset.parameters.size()) j["parameters"] = preset.parameters;
    j["settings"] = preset.settings;
}

void from_json(const json& j, PresetDescriptor& preset) {
    preset.voice = j.at("voice").get<PatchDescriptor>();
    preset.effect = j.at("effect").get<PatchDescriptor>();
    if (j.count("parameters")) preset.parameters = j.at("parameters").get<vector<PresetParameterDescriptor>>();
    if (j.count("settings")) preset.settings = j.at("settings").get<PresetSettings>();
}

void to_json(json& j, const PresetSettings& settings) {
    j["polyphony"] = settings.polyphony;
    j["multicore"] = settings.multicore;
    if (settings.noteActivationPolicy != c_NoteActivationPolicyDefault) j["noteActivationPolicy"] = settings.noteActivationPolicy;
    if (settings.noteStealPolicy != c_NoteStealPolicyDefault) j["noteStealPolicy"] = settings.noteStealPolicy;
    if (settings.noteReactivationPolicy != c_NoteReactivationPolicyDefault) j["noteReactivationPolicy"] = settings.noteReactivationPolicy;
    if (settings.legatoMode != c_LegatoModeDefault) j["legatoMode"] = settings.legatoMode;
}

void from_json(const json& j, PresetSettings& settings) {
    if (j.count("polyphony")) settings.polyphony = j.at("polyphony").get<int>();
    if (j.count("multicore")) settings.multicore = j.at("multicore").get<bool>();
    if (j.count("noteStealPolicy")) settings.noteStealPolicy = j.at("noteStealPolicy").get<NoteStealPolicy>();
    if (j.count("noteReactivationPolicy")) settings.noteReactivationPolicy = j.at("noteReactivationPolicy").get<NoteReactivationPolicy>();
    if (j.count("legatoMode")) settings.legatoMode = j.at("legatoMode").get<LegatoMode>();
    if (j.count("noteActivationPolicy")) settings.noteActivationPolicy = j.at("noteActivationPolicy").get<NoteActivationPolicy>();
}

}
