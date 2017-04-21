#include "design_json.hpp"

/*
#include "nlohmann/json.hpp"
using nlohmann::json;
*/

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
    j[0] = mpc.from.module;
    j[1] = mpc.from.port;
    j[2] = mpc.to.module;
    j[3] = mpc.to.port;
}

void from_json(const json& j, ModulePortConnection& mpc) {
    mpc.from.module = j.at(0).get<std::string>();
    mpc.from.port = j.at(1).get<std::string>();
    mpc.to.module = j.at(2).get<std::string>();
    mpc.to.port = j.at(3).get<std::string>();
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

void to_json(json& j, const ComponentDescriptor& cgd) {
    j["modules"] = cgd.graph.modules;
    j["connections"] = cgd.graph.connections;
    j["values"] = cgd.graph.values;
    j["input"] = cgd.input;
    j["output"] = cgd.output;
}

void from_json(const json& j, ComponentDescriptor& cgd) {
    cgd.graph.modules = j.at("modules").get<std::vector<ModuleVariable>>();
    cgd.graph.connections = j.at("connections").get<std::vector<ModulePortConnection>>();
    cgd.graph.values = j.at("values").get<std::vector<ModulePortValue>>();
    cgd.input = j.at("input").get<std::vector<ModulePort>>();
    cgd.output = j.at("output").get<std::vector<ModulePort>>();
}

void to_json(json& j, const PatchDescriptor& cgd) {
    j["voice"] = cgd.voiceGraph;
    j["effect"] = cgd.effectGraph;
}

void from_json(const json& j, PatchDescriptor& cgd) {
    cgd.voiceGraph = j.at("voice");
    cgd.effectGraph = j.at("effect");
}

// I hate myself for doing this ...

std::string prettydump_(const ConnectionGraphDescriptor& cgd){
    // rigid stupid pretty dump thing
    // as the json.hpp has too simplistic pretty printing
    std::stringstream ss;

    int i = 0;
    ss << "  \"modules\" : [";
    i = cgd.modules.size();
    for (const auto& m : cgd.modules) {
        ss << "\n    " << json(m);
        if (--i != 0) ss << ",";
    }
    ss << "\n  ],\n";

    ss << "  \"connections\" : [";
    i = cgd.connections.size();
    for (const auto& c : cgd.connections) {
        ss << "\n    " << json(c);
        if (--i != 0) ss << ",";
    }
    ss << "\n  ],\n";

    ss << "  \"values\" : [";
    i = cgd.values.size();
    for (const auto& v : cgd.values) {
        ss << "\n    " << json(v);
        if (--i != 0) ss << ",";
    }
    ss << "\n  ]\n";

    return ss.str();
}

std::string prettydump(const ConnectionGraphDescriptor& cgd)
{
    std::stringstream ss;
    ss << u8"{" << std::endl;
    ss << prettydump_(cgd);
    ss << "}" << std::endl;
    return ss.str();
}

std::string prettydump(const ComponentDescriptor& cgd){
    std::stringstream ss;
    ss << u8"{" << std::endl;

    int i = 0;
    ss << "  \"input\" : [";
    i = cgd.input.size();
    for (const auto& m : cgd.input) {
        ss << "\n    " << json(m);
        if (--i != 0) ss << ",";
    }
    ss << "\n  ],\n";

    ss << "  \"output\" : [";
    i = cgd.input.size();
    for (const auto& m : cgd.output) {
        ss << "\n    " << json(m);
        if (--i != 0) ss << ",";
    }
    ss << "\n  ],\n";

    ss << prettydump_(cgd.graph);

    ss << "}" << std::endl;
    return ss.str();
}

std::string prettydump(const PatchDescriptor& cgd){
    std::stringstream ss;
    ss << u8"{" << std::endl;
    ss << "\"voice\" : " << prettydump(cgd.voiceGraph);
    ss << ",\"effect\" : " << prettydump(cgd.effectGraph);
    ss << "}" << std::endl;
    return ss.str();
}

}
