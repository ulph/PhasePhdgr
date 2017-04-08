add_subdirectory(connectiongraph)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SRC
    Effect.hpp
    MPEVoice.hpp
    MPEVoice.cpp
    Synth.cpp
    SynthVoice.cpp
    SynthVoice.hpp
    VoiceBus.cpp
    VoiceBus.hpp
)

add_library(phasephckr ${SRC})

set_target_properties(phasephckr PROPERTIES COMPILE_FLAGS "-fPIC")

install(TARGETS phasephckr DESTINATION lib)
