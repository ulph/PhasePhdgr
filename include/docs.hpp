#ifndef DOCS_HPP
#define DOCS_HPP

#include <vector>
#include <string>

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
        std::vector<ModuleDoc> docs;
        const ComponentRegister & cp;
    public:
        Doc(const ComponentRegister & cp);
        const std::vector<ModuleDoc> & get();
    };
}

#endif