add_subdirectory(connectiongraph)
add_subdirectory(modules)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(connectiongraph modules)
set(SRC
    ${CMAKE_SOURCE_DIR}/include/PhasePhckr.h
    MPEVoice.hpp
    MPEVoice.cpp
    Synth.cpp
    SynthVoice.cpp
    SynthVoice.hpp
    EffectChain.cpp
    EffectChain.hpp
    BusModules.hpp
    VoiceBus.cpp
    VoiceBus.hpp
)

add_library(phasephckr ${SRC})

set_target_properties(phasephckr PROPERTIES COMPILE_FLAGS "-fPIC")

install(TARGETS phasephckr DESTINATION lib)
