#include <phasephckr.hpp>

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
        cg.makeModuleDocs(newDoc);
        for(const auto &d : newDoc){
            add(d);
        }
        // ... note, components are added later, as they are (partly) read from files
    }

    const std::map<std::string, ModuleDoc> & Doc::get() const {
        return docs;
    }

    void Doc::add(const ModuleDoc & d) {
        docs[d.type] = d;
    }

    ModuleDoc Doc::makeBusModuleDoc(const vector<PadDescription>& ports, bool isInput){
        auto m = BusModule(ports, isInput);
        return m.makeDoc();
    }

}
