#pragma once

#include "phasephckr/design.hpp"

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

    void to_json(json& j, const ComponentDescriptor& cgd);
    void from_json(const json& j, ComponentDescriptor& cgd);

    void to_json(json& j, const ModulePosition& xy);
    void from_json(const json& j, ModulePosition& xy);

    void to_json(json& j, const PatchDescriptor& cgd);
    void from_json(const json& j, PatchDescriptor& cgd);

    void to_json(json& j, const PatchParameterDescriptor& param);
    void from_json(const json& j, PatchParameterDescriptor& param);

    void to_json(json& j, const PresetParameterDescriptor& param);
    void from_json(const json& j, PresetParameterDescriptor& param);

    void to_json(json& j, const VoiceSettings& param);
    void from_json(const json& j, VoiceSettings& param);

    void to_json(json& j, const PresetDescriptor& preset);
    void from_json(const json& j, PresetDescriptor& preset);

}
