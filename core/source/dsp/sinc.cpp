#include "sinc.hpp"

#define SINC_MAKE_TEMPLATE_SHIT(N) \
const FractionalSincTable< ##N## > c_fractionalSincTable_ ##N## (true); \
template <> const FractionalSincTable<##N##> & getFractionalSincTable() { \
    return c_fractionalSincTable_##N##; \
}

SINC_MAKE_TEMPLATE_SHIT(4);
SINC_MAKE_TEMPLATE_SHIT(8);
SINC_MAKE_TEMPLATE_SHIT(16);
SINC_MAKE_TEMPLATE_SHIT(32);
