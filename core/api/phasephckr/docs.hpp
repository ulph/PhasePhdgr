#pragma once

#include <vector>
#include <string>
#include <map>

#include "design.hpp"

class Module;

namespace PhasePhckr {

    struct ModuleDoc {
        string type;
        std::vector<PadDescription> inputs;
        std::vector<PadDescription> outputs;
        std::string docString;
        void fromModule(const Module* module);
        void fromBusModulePorts(const vector<PadDescription>& ports, bool isInput);
    };

    class Doc {
    private:
        std::map<std::string, ModuleDoc> docs;
    public:
        Doc();
        const std::map<std::string, ModuleDoc> & get() const;
        void add(const ModuleDoc & d);
    };
}
