#include "sinc.hpp"

#define MAKE_SINC_TABLE(N) \
const FractionalSincTable< N > c_fractionalSincTable_##N (true); \
template <> const FractionalSincTable< N > & getFractionalSincTable() { \
    return c_fractionalSincTable_##N; \
}

MAKE_SINC_TABLE(5);
MAKE_SINC_TABLE(9);
MAKE_SINC_TABLE(17);
MAKE_SINC_TABLE(33);
