#include "design_json.hpp"
#include "EffectChain.hpp"
#include <iostream>

using namespace PhasePhckr;

json test(std::string what, json j1, json j2) {
    json diff = json::diff(j1, j2);
    bool isOk = diff.size() == 0;
    std::cout << what << " " << (isOk ? "ok" : "nok") << std::endl;
    return diff;
}

int main(int argc, char *argv[])
{
    ModuleVariable mv = { "a name", "some type" };
    if (0 != test("ModuleVariable", mv, (ModuleVariable)json(mv)).size()) {
        return -1;
    }

    ModulePort mp = { "a module", "a port" };
    if (0 != test("ModulePort", mp, (ModulePort)json(mp)).size()) {
        return -1;
    }

    ModulePortValue mpv = { { "some module", "some port" }, 42.0f };
    if (0 != test("ModulePortValue", mpv, (ModulePortValue)json(mpv)).size()) { 
        return -1;
    }

    ModulePortConnection mpc = { {"from module", "from port"}, {"to module", "to port"} };
    if (0 != test("ModulePortConnection", mpc, (ModulePortConnection)json(mpc)).size()) {
        return -1;
    }

    ConnectionGraphDescriptor fxchain = getExampleFxChain();
    if (0 != test("ConnectionGraphDescriptor", fxchain, (ConnectionGraphDescriptor)(json(fxchain))).size()) {
        return -1;
    }

    ComponentDescriptor component = getExampleComponent();
    if (0 != test("ComponentDescriptor", component, (ComponentDescriptor)(json(component))).size()) {
        return -1;
    }

    return 0;
}