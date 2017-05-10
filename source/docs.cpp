#include "docs.hpp"
#include "moduleregister.hpp"
#include "connectiongraph.hpp"
#include "module.hpp"
#include "components.hpp"
#include "busmodules.hpp"

namespace PhasePhckr {

    Doc::Doc()
    {
        std::vector<ModuleDoc> newDoc;
        ConnectionGraph cg;
        ModuleRegister::registerAllModules(cg);
        cg.makeModuleDocs(newDoc);
        for(const auto &d : newDoc){
            docs[d.type] = d;
        }
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
