#include "docs.hpp"
#include "moduleregister.hpp"
#include "connectiongraph.hpp"
#include "module.hpp"

namespace PhasePhckr {

    Doc::Doc() {
        ConnectionGraph cg;
        ModuleRegister::registerAllModules(cg);
        cg.makeModuleDocs(moduleDocs);
    }

    const std::vector<ModuleDoc> & Doc::get() {
        return moduleDocs;
    }

}