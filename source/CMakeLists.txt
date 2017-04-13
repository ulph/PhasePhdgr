option(MULTITHREADING OFF)
if(${MULTITHREADING})
    add_definitions(-DMULTITHREADED=1)
else()
    add_definitions(-DMULTITHREADED=0)
endif()

add_subdirectory(connectiongraph)
add_subdirectory(modules)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(connectiongraph modules)
set(SRC
    ${CMAKE_SOURCE_DIR}/include/PhasePhckr.h
    MPEVoice.cpp
    Synth.cpp
    SynthVoice.cpp
    SynthVoice.hpp
    EffectChain.cpp
    EffectChain.hpp
    BusModules.hpp
    VoiceBus.cpp
    VoiceBus.hpp
    design.hpp
    design.cpp
    parameters.hpp
    scope.cpp
)

if(${USING_NLOHMANN_JSON})
    list(APPEND SRC design_json.hpp)
endif()

add_library(phasephckr ${SRC})
target_link_libraries (phasephckr ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(phasephckr PROPERTIES COMPILE_FLAGS "-fPIC")

install(TARGETS phasephckr DESTINATION lib)
