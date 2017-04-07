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