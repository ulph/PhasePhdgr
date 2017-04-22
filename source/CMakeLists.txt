option(MULTITHREADING OFF)
if(${MULTITHREADING})
    add_definitions(-DMULTITHREADED=1)
else()
    add_definitions(-DMULTITHREADED=0)
endif()

add_subdirectory(connectiongraph)
add_subdirectory(dsp)
add_subdirectory(modules)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(connectiongraph modules dsp)
set(SRC
    MPEVoice.cpp
    Synth.cpp
    SynthVoice.cpp
    SynthVoice.hpp
    EffectChain.cpp
    EffectChain.hpp
    examples.cpp
    BusModules.hpp
    VoiceBus.cpp
    VoiceBus.hpp
    design.cpp
    parameters.hpp
    scope.cpp
)

if(${USING_NLOHMANN_JSON})
    list(APPEND SRC design_json.cpp)
endif()

add_library(phasephckr_obj OBJECT ${SRC})
add_library(phasephckr $<TARGET_OBJECTS:phasephckr_obj>)
set_property(TARGET phasephckr PROPERTY FOLDER targets/lib)

target_link_libraries(phasephckr ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(phasephckr PROPERTIES COMPILE_FLAGS "-fPIC")
install(TARGETS phasephckr DESTINATION lib)
