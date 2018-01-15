#pragma once

#include <string>
#include <phasephckr.hpp>
using namespace PhasePhckr;

const string rootMarker = "root";

bool validRootComponent(PatchDescriptor * patch, ComponentDescriptor* rootComponent);
