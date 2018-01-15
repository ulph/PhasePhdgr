#include "CommonEditor.hpp"

bool validRootComponent(PatchDescriptor * patch, ComponentDescriptor* rootComponent) {
    if (!rootComponent) return false;
    bool validRootComponent = rootComponent == &patch->root;
    if (!validRootComponent) {
        for (const auto& kv : patch->components) {
            if (&(kv.second) == rootComponent) validRootComponent = true;
        }
    }
    return validRootComponent;
}