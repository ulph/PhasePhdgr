#pragma once

#include "design.hpp"
#include <string.h>

#include "design.hpp"
#include "docs.hpp"

using namespace std;

namespace PhasePhckr {

    struct ComponentDescriptor;

    class ComponentRegister {      
    private:
        std::map<std::string, ComponentDescriptor> r;
        void registerFactoryComponents();
    public:
        ComponentRegister(){
            registerFactoryComponents();
        }
        bool registerComponent(std::string name, const ComponentDescriptor & desc);
        bool getComponent(std::string name, ComponentDescriptor & desc) const;
        static void makeComponentDoc(const string &type, const ComponentDescriptor & cmp, ModuleDoc &doc);
        bool makeComponentDoc(const string &name, ModuleDoc &doc) const;
        void makeComponentDocs(Doc &ref) const;
        const map<std::string, ComponentDescriptor> & all() const {
            return r;
        }
    };

}
