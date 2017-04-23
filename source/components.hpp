#pragma once

#include "design.hpp"
#include <string.h>

namespace PhasePhckr {
    bool registerComponent(std::string name, const ComponentDescriptor & desc);
    bool getComponent(std::string name, ComponentDescriptor & desc);
}
