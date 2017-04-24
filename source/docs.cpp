#include "docs.hpp"
#include "moduleregister.hpp"
#include "connectiongraph.hpp"
#include "module.hpp"
#include "components.hpp"

namespace PhasePhckr {

    Doc::Doc(const ComponentRegister & cp) 
        : cp(cp) 
    {
        std::vector<ModuleDoc> newDoc;
        ConnectionGraph cg;
        ModuleRegister::registerAllModules(cg);
        cg.makeModuleDocs(newDoc);
        cp.makeComponentDocs(newDoc);

        for(const auto &d : newDoc){
            docs[d.type] = d;
        }
    }

    const std::map<std::string, ModuleDoc> & Doc::get() const {
        return docs;
    }

}
