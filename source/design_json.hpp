#pragma once

#include "design.hpp"
#include <sstream>

#include "nlohmann/json.hpp"
using nlohmann::json;

namespace PhasePhckr {

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

    std::string prettydump(ConnectionGraphDescriptor& cgd) {
        // rigid stupid pretty dump thing
        // as the json.hpp has too simplistic pretty printing
        int i = 0;
        std::stringstream ss;
        ss << u8"{" << std::endl;

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

        ss << "}" << std::endl;

        return ss.str().c_str();
    }

}
