include_directories(components design docs synth utilities)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/phasephdgr_version.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/phasephdgr_version.cpp @ONLY)

set(SRC
    ${CMAKE_CURRENT_BINARY_DIR}/phasephdgr_version.cpp

    components/components.cpp

    design/busmodules.hpp
    design/design.cpp

    docs/docs.cpp

    synth/synth.cpp
    synth/synthvoice.cpp
    synth/synthvoice.hpp
    synth/effectchain.cpp
    synth/effectchain.hpp
    synth/voicebus.cpp
    synth/voicebus.hpp
    synth/parameters.hpp

    utilities/examples.cpp
    utilities/json.cpp
    utilities/scope.cpp
)

add_library(phasephdgr_obj OBJECT ${SRC})
set_target_properties(phasephdgr_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(phasephdgr_obj PROPERTIES FOLDER core)
