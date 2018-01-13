#include "phasephckr/docs.hpp"

#include "moduleregister.hpp"
#include "connectiongraph.hpp"
#include "moduleaccessor.hpp"
#include "busmodules.hpp"

#include "pluginloader.hpp"

namespace PhasePhckr {

    void ModuleDoc::fromModule(const Module* module) {
        type = ModuleAccessor::getName(*module);
        docString = module->docString();
        for (const auto p : ModuleAccessor::getInputs(*module)) {
            PhasePhckr::PadDescription pd;
            pd.name = p.name;
            pd.unit = p.unit;
            pd.defaultValue = p.value;
            inputs.push_back(pd);  
        }
        for (const auto p : ModuleAccessor::getOutputs(*module)) {
            PhasePhckr::PadDescription pd;
            pd.name = p.name;
            pd.unit = p.unit;
            pd.defaultValue = p.value;
            outputs.push_back(pd);
        }
    }

    void ModuleDoc::fromBusModulePorts(const vector<PadDescription>& ports, bool isInput) {
        BusModule m(ports, isInput);
        fromModule(&m);
        if (isInput) inputs.clear();
        else outputs.clear();
    }

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
    }

    const std::map<std::string, ModuleDoc> & Doc::get() const {
        return docs;
    }

    void Doc::add(const ModuleDoc & d) {
        docs[d.type] = d;
    }
}
