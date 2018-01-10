#include "phasephckr/docs.hpp"

#include "moduleregister.hpp"
#include "connectiongraph.hpp"
#include "module.hpp"
#include "busmodules.hpp"

namespace PhasePhckr {

    Doc::Doc()
    {        
        // standard module docs
        std::vector<ModuleDoc> newDoc;
        ConnectionGraph cg;
        ModuleRegister::registerAllModules(cg);
        for(const auto &d : newDoc){
            add(d);
        }
    }

    const std::map<std::string, ModuleDoc> & Doc::get() const {
        return docs;
    }

    void Doc::add(const ModuleDoc & d) {
        docs[d.type] = d;
    }
}
