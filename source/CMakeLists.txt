add_subdirectory(connectiongraph)


set(SRC
    PhasePhckr.cpp
    PhasePhckr.h
    SynthVoices/ExSynthVoice.h 
    SynthVoices/ExSynthVoice.cpp
    Components/CamelEnvelope.cpp
    Components/CamelEnvelope.h
)

add_library(phasephckr ${SRC})

set_target_properties(phasephckr PROPERTIES COMPILE_FLAGS "-fPIC")

install(TARGETS phasephckr DESTINATION lib)
install(FILES PhasePhckr.h DESTINATION include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})