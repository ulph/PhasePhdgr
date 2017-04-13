#pragma once

#include "design.hpp"

#include "nlohmann/json.hpp"
using nlohmann::json;

namespace PhasePhckr {

    void to_json(json& j, const ModuleVariable& mv) {
        j[0] = mv.name;
        j[1] = mv.type;
    }

    void from_json(const json& j, ModuleVariable& mv) {
        mv.name = j[0].get<std::string>();
        mv.type = j[1].get<std::string>();
    }

    void to_json(json& j, const ModulePort& mp) {
        j[0] = mp.module;
        j[1] = mp.port;
    }

    void from_json(const json& j, ModulePort& mp) {
        mp.module = j[0].get<std::string>();
        mp.port = j[1].get<std::string>();
    }

    void to_json(json& j, const ModulePortValue& mpv) {
        j["target"] = mpv.target;
        j["value"] = mpv.value;
    }

    void from_json(const json& j, ModulePortValue& mpv) {
        mpv.target = j["target"];
        mpv.value = j["value"];
    }

    void to_json(json& j, const ModulePortConnection& mpc) {
        j["from"] = mpc.from;
        j["to"] = mpc.to;
    }

    void from_json(const json& j, ModulePortConnection& mpc) {
        mpc.from = j["from"];
        mpc.to = j["to"];
    }

    void to_json(json& j, const ConnectionGraphDescriptor& cgd) {
        j["modules"] = cgd.modules;
        j["connections"] = cgd.connections;
        j["values"] = cgd.values;
    }

    void from_json(const json& j, ConnectionGraphDescriptor& cgd) {
        cgd.modules = j["modules"].get<std::vector<ModuleVariable>>();
        cgd.connections = j["connections"].get<std::vector<ModulePortConnection>>();
        cgd.values = j["values"].get<std::vector<ModulePortValue>>();
    }

}
