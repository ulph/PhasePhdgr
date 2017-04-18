set(SRC
    abs.cpp
    abs.hpp
    biquad.cpp
    biquad.hpp
    bleposc.cpp
    bleposc.hpp
    camelenvelope.cpp
    camelenvelope.hpp
    clamp.cpp
    clamp.hpp
    clampinv.cpp
    clampinv.hpp
    crossfade.cpp
    crossfade.hpp
    delay.cpp
    delay.hpp
    foldback.cpp
    foldback.hpp
    rlc.cpp
    rlc.hpp
    moduleregister.hpp
    mul.cpp
    mul.hpp
    noise.cpp
    noise.hpp
    phase.cpp
    phase.hpp
    quant8.cpp
    quant8.hpp
    saturatoratan.cpp
    saturatoratan.hpp
    scaleshift.cpp
    scaleshift.hpp
    sine.cpp
    sine.hpp
    square.cpp
    square.hpp
    sympow.cpp
    sympow.hpp
)

include_directories(../connectiongraph)
include_directories(../dsp)
add_library(modules ${SRC})

set_target_properties(modules PROPERTIES COMPILE_FLAGS "-fPIC")

install(TARGETS modules DESTINATION lib)
