#pragma once

#include <string.h>
#include <map>

using namespace std;

namespace PhasePhckr {

    struct PadDescription;
    struct ComponentDescriptor;
    struct ModuleDoc;
    class Doc;

    class ComponentRegister {
    private:
        map<string, ComponentDescriptor> r;
        void registerFactoryComponents();
    public:
        ComponentRegister(){
            registerFactoryComponents();
        }
        bool registerComponent(string name, const ComponentDescriptor & desc);
        bool getComponent(string name, ComponentDescriptor & desc) const;
        static void makeComponentDoc(const string &type, const ComponentDescriptor & cmp, ModuleDoc &doc);
        bool makeComponentDoc(const string &name, ModuleDoc &doc) const;
        void makeComponentDocs(Doc &doc) const;
        const map<string, ComponentDescriptor> & all() const {
            return r;
        }
    };

}
