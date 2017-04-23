#include "docs.hpp"
#include "moduleregister.hpp"
#include "connectiongraph.hpp"
#include "module.hpp"
#include "components.hpp"

namespace PhasePhckr {

    Doc::Doc(const ComponentRegister & cp) 
        : cp(cp) 
    {
        ConnectionGraph cg;
        ModuleRegister::registerAllModules(cg);
        cg.makeModuleDocs(docs);
        cp.makeComponentDocs(docs);
    }

    const std::vector<ModuleDoc> & Doc::get() {
        return docs;
    }

}