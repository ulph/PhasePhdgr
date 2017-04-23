#pragma once

#include "design.hpp"

#include "nlohmann/json.hpp"
using nlohmann::json;

namespace PhasePhckr {

    void to_json(json& j, const ModuleVariable& mv);
    void from_json(const json& j, ModuleVariable& mv);

    void to_json(json& j, const ModulePort& mp);
    void from_json(const json& j, ModulePort& mp);

    void to_json(json& j, const ModulePortValue& mpv);
    void from_json(const json& j, ModulePortValue& mpv);

    void to_json(json& j, const ModulePortConnection& mpc);
    void from_json(const json& j, ModulePortConnection& mpc);

    void to_json(json& j, const ConnectionGraphDescriptor& cgd);
    void from_json(const json& j, ConnectionGraphDescriptor& cgd);

}
