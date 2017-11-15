#include <string.h>
#include "delay.hpp"
#include "sinc.hpp"
#include <assert.h>
namespace DelayFactory {

    const FractionalSincTable<4, c_fractions> c_fractionalSincTable_4(true);
    const FractionalSincTable<8, c_fractions> c_fractionalSincTable_8(true);
    const FractionalSincTable<16, c_fractions> c_fractionalSincTable_16(true);
    const FractionalSincTable<32, c_fractions> c_fractionalSincTable_32(true);

    template <> const FractionalSincTable<4, c_fractions> & getFractionalSincTable() {
        return c_fractionalSincTable_4;
    }

    template <> const FractionalSincTable<8, c_fractions> & getFractionalSincTable() {
        return c_fractionalSincTable_8;
    }

    template <> const FractionalSincTable<16, c_fractions> & getFractionalSincTable() {
        return c_fractionalSincTable_16;
    }

    template <> const FractionalSincTable<32, c_fractions> & getFractionalSincTable() {
        return c_fractionalSincTable_32;
    }

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