set(SRC
    biquad.cpp
    blitosc.cpp
    env.cpp
    svf.cpp
    map.cpp
    conversion.cpp
    crossfade.cpp
    div.cpp
    delay.cpp
    foldback.cpp
    rlc.cpp
    mul.cpp
    noise.cpp
    phase.cpp
    quantize.cpp
    sine.cpp
    sympow.cpp
    samphold.cpp
    logic.cpp
    zdf.cpp
)

add_library(modules_obj OBJECT ${SRC})
set_target_properties(modules_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(modules_obj PROPERTIES FOLDER core)
