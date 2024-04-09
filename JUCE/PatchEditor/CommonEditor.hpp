#pragma once

#include <string>
#include <phasephdgr.hpp>
using namespace PhasePhdgr;

const string rootMarker = "root";

bool validRootComponent(PatchDescriptor * patch, ComponentDescriptor* rootComponent);
