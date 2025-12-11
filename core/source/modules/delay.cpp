#include "delay.hpp"
#include <assert.h>

namespace DelayFactory {
    Module* (*makeFactory(int numFractions)) (void) {
        auto* factory = Delay<5>::factory;
        switch (numFractions) {
        case 4:
            factory = Delay<5>::factory; break;
        case 8:
            factory = Delay<9>::factory; break;
        case 16:
            factory = Delay<17>::factory; break;
        case 32:
            factory = Delay<33>::factory; break;
        default:
            assert(0); break;
        }
        return factory;
    }
}
