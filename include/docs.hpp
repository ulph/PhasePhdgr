#ifndef DOCS_HPP
#define DOCS_HPP

#include "connectiongraph.hpp"
#include "module.hpp"

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
    void RefreshDocs();
    const std::vector<ModuleDoc> & GetModuleDocs();
}

#endif