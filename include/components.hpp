#pragma once

#include "design.hpp"
#include <string.h>

#include "design.hpp"

namespace PhasePhckr {

    struct ComponentDescriptor;

    class ComponentRegister {
    private:
        std::map<std::string, ComponentDescriptor> r;
    public:
        bool registerComponent(std::string name, const ComponentDescriptor & desc);
        bool getComponent(std::string name, ComponentDescriptor & desc) const;
        void registerFactoryComponents();
    };

}
