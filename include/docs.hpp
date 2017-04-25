#ifndef DOCS_HPP
#define DOCS_HPP

#include <vector>
#include <string>
#include <map>

struct PadDescription {
    float value;
    std::string name;
    std::string unit;
};

struct ModuleDoc {
    std::string type;
    std::vector<PadDescription> inputs;
    std::vector<PadDescription> outputs;
    std::string docString;
};

namespace PhasePhckr {
    class ComponentRegister;

    class Doc {
    private:
        std::map<std::string, ModuleDoc> docs;
        const ComponentRegister & cp;
    public:
        Doc(const ComponentRegister & cp);
        const std::map<std::string, ModuleDoc> & get() const;
        void add(const ModuleDoc & d);
    };

    ModuleDoc getVoiceBusInputDoc();
    ModuleDoc getVoiceBusOutputDoc();
    ModuleDoc getEffectBusInputDoc();
    ModuleDoc getEffectBusOutputDoc();

}

#endif
