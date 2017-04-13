option(MULTITHREADING OFF)
if(${MULTITHREADING})
    add_definitions(-DMULTITHREADED=1)
else()
    add_definitions(-DMULTITHREADED=0)
endif()

add_subdirectory(connectiongraph)
add_subdirectory(modules)

option(USING_NLOHMANN_JSON OFF)
if(${USING_NLOHMANN_JSON})
    set(JSON_INCLUDE_DIR "" CACHE FILEPATH "Install dir of https://nlohmann.github.io/json/")
    include_directories(${JSON_INCLUDE_DIR}/include)
    if(NOT EXISTS "${JSON_INCLUDE_DIR}/include/nlohmann/json.hpp")
        error( "Cannot find nlohmann/json.hpp" )
    endif()
    add_definitions(-DUSING_NLOHMANN_JSON=1)
else()
    add_definitions(-DUSING_NLOHMANN_JSON=0)
endif()

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

add_library(phasephckr ${SRC})
target_link_libraries (phasephckr ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(phasephckr PROPERTIES COMPILE_FLAGS "-fPIC")

install(TARGETS phasephckr DESTINATION lib)
