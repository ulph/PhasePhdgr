#include <string.h>
#include "delay.hpp"
#include "sinc.hpp"
#include <assert.h>

namespace DelayFactory {
    Module* (*makeFactory(int numFractions)) (void) {
        auto* factory = Delay<32>::factory;
        switch (numFractions) {
        case 4:
            factory = Delay<4>::factory; break;
        case 8:
            factory = Delay<8>::factory; break;
        case 16:
            factory = Delay<16>::factory; break;
        case 32:
            factory = Delay<32>::factory; break;
        default:
            assert(0); break;
        }
        return factory;
    }
}
