set(SRC
    biquad.cpp
    biquad.hpp
    blitosc.cpp
    blitosc.hpp
    env.cpp
    env.hpp
    svf.cpp
    svf.hpp
    map.cpp
    map.hpp
    conversion.cpp
    conversion.hpp
    crossfade.cpp
    crossfade.hpp
    div.cpp
    div.hpp
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
    quantize.cpp
    quantize.hpp
    sine.cpp
    sine.hpp
    sympow.cpp
    sympow.hpp
    samphold.cpp
    samphold.hpp
    logic.cpp
    logic.hpp
    inlines.hpp
    zdf.cpp
    zdf.hpp
    unarymath.hpp
)

add_library(modules_obj OBJECT ${SRC})
set_target_properties(modules_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(modules_obj PROPERTIES FOLDER core)
