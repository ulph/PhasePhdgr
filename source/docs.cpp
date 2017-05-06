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

    ModuleDoc getVoiceBusInputDoc() {
        VoiceInputBus m;
        m.setName(c_VoiceInput.type);
        return m.makeDoc();
    }

    ModuleDoc getVoiceBusOutputDoc() {
        StereoOutBus m;
        m.setName(c_VoiceOutput.type);
        return m.makeDoc();
    }

    ModuleDoc getEffectBusInputDoc() {
        EffectInputBus m;
        m.setName(c_EffectInput.type);
        return m.makeDoc();
    }

    ModuleDoc getEffectBusOutputDoc() {
        StereoOutBus m;
        m.setName(c_EffectOutput.type);
        return m.makeDoc();
    }

}
