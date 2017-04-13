#include "design_json.hpp"
#include "EffectChain.hpp"
#include <iostream>

using namespace PhasePhckr;

int test(std::string what, json j1, json j2) {
    bool isOk = j1 == j2;
    std::cout << what << " " << (isOk ? "ok" : "nok") << std::endl;
    return isOk ? 0 : -1;
}

int main(int argc, char *argv[])
{
    ModuleVariable mv = { "a name", "some type" };
    if (test("ModuleVariable", mv, (ModuleVariable)json(mv))) return -1;

    ModulePort mp = { "a module", "a port" };
    if (test("ModulePort", mp, (ModulePort)json(mp))) return -1;

    ModulePortValue mpv = { { "some module", "some port" }, 42.0f };
    if (test("ModulePortValue", mpv, (ModulePortValue)json(mpv))) return -1;

    ModulePortConnection mpc = { {"from module", "from port"}, {"to module", "to port"} };
    if (test("ModulePortConnection", mpc, (ModulePortConnection)json(mpc))) return -1;

    ConnectionGraphDescriptor fxchain = getExFxChain();
    if (test("ConnectionGraphDescriptor", fxchain, (ConnectionGraphDescriptor)(json(fxchain)))) return -1;

    return 0;
}