#include "sinc.hpp"

#define MAKE_SINC_TABLE(N) \
const FractionalSincTable< N > c_fractionalSincTable_##N (true); \
template <> const FractionalSincTable< N > & getFractionalSincTable() { \
    return c_fractionalSincTable_##N; \
}

MAKE_SINC_TABLE(4);
MAKE_SINC_TABLE(8);
MAKE_SINC_TABLE(16);
MAKE_SINC_TABLE(32);
