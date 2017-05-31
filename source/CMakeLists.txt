add_subdirectory(connectiongraph)
add_subdirectory(dsp)
add_subdirectory(modules)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(connectiongraph modules dsp)
set(SRC
    mpevoice.cpp
    synth.cpp
    synthvoice.cpp
    synthvoice.hpp
    effectchain.cpp
    effectchain.hpp
    examples.cpp
    busmodules.hpp
    voicebus.cpp
    voicebus.hpp
    design.cpp
    parameters.hpp
    scope.cpp
    components.cpp
    docs.cpp
)

if(${USING_NLOHMANN_JSON})
    list(APPEND SRC design_json.cpp)
endif()

add_library(phasephckr_obj OBJECT ${SRC})
set_target_properties(phasephckr_obj PROPERTIES COMPILE_FLAGS "-fPIC")
