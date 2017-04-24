#include "docs.hpp"
#include "moduleregister.hpp"
#include "connectiongraph.hpp"
#include "module.hpp"
#include "components.hpp"
#include "busmodules.hpp"

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

    void Doc::add(const ModuleDoc & d) {
        docs[d.type] = d;
    }

    ModuleDoc getVoiceBusInputDoc() {
        VoiceInputBus m;
        m.setName("_VOICEINPUT");
        return m.makeDoc();
    }

    ModuleDoc getVoiceBusOutputDoc() {
        StereoBus m;
        m.setName("_VOICEOUTPUT");
        return m.makeDoc();
    }

    ModuleDoc getEffectBusInputDoc() {
        EffectInputBus m;
        m.setName("_EFFECTINPUT");
        return m.makeDoc();
    }

    ModuleDoc getEffectBusOutputDoc() {
        StereoBus m;
        m.setName("_EFFECTOUTPUT");
        return m.makeDoc();
    }

}
