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
    class Doc {
    private:
        void RefreshDocs();
        std::vector<ModuleDoc> moduleDocs;
    public:
        Doc();
        const std::vector<ModuleDoc> & get();
    };
}

#endif