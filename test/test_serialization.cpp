#include "design_json.hpp"
#include "EffectChain.hpp"
#include <iostream>

using namespace PhasePhckr;

int main(int argc, char *argv[])
{
    ModuleVariable mv = { "a name", "some type" };
    json mvj = mv;
    std::cout << mvj << std::endl;
    ModuleVariable mv2 = mvj;
    std::cout << json(mv2) << std::endl << std::endl;

    ModulePort mp = { "a module", "a port" };
    json mpj = mp;
    std::cout << mpj << std::endl;
    ModulePort mp2 = mpj;
    std::cout << json(mp2) << std::endl << std::endl;

    ModulePortValue mpv = { { "some module", "some port" }, 42.0f };
    json mpvj = mpv;
    std::cout << mpvj << std::endl;
    ModulePortValue mpv2 = mpvj;
    std::cout << json(mpv2) << std::endl << std::endl;

    ModulePortConnection mpc = { {"from module", "from port"}, {"to module", "to port"} };
    json mpcj = mpc;
    std::cout << mpcj << std::endl;
    ModulePortConnection mpc2 = mpcj;
    std::cout << json(mpc2) << std::endl << std::endl;

    ConnectionGraphDescriptor fxchain = getExFxChain();
    json fxchainj = fxchain;
    std::cout << fxchainj << std::endl;
    ConnectionGraphDescriptor fxchain2 = fxchainj;
    std::cout << json(fxchain2) << std::endl << std::endl;

    return 0;
}