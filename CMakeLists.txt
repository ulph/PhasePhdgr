project(phasephckr)
cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 14)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
														 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(BUILD_VST OFF)

add_subdirectory(core)

set(API_INC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/phasephckr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/components.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/docs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/design.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/design_json.hpp
)

if(${BUILD_VST})

    option(FORCE_FTZ_DAZ OFF)
    if(${FORCE_FTZ_DAZ})
        add_definitions(-DFORCE_FTZ_DAZ=1)
    else()
        add_definitions(-DFORCE_FTZ_DAZ=0)
    endif()

    option(INTERCEPT_STD_STREAMS ON)
    if(${INTERCEPT_STD_STREAMS})
        add_definitions(-DINTERCEPT_STD_STREAMS=1)
    else()
        add_definitions(-DINTERCEPT_STD_STREAMS=0)
    endif()

    # VST3 SDK stuff
    set(VST3_SDK_ROOT "" CACHE FILEPATH "VST3 SDK root")
    include_directories(${VST3_SDK_ROOT})

    # JUCE stuff
    set(JUCE_SDK_ROOT "" CACHE FILEPATH "JUCE5 SDK root")
    add_subdirectory(JUCE)

    # our + JUCE object libs
    set( PHASEPHCKR_SRC
        $<TARGET_OBJECTS:connectiongraph_obj>
        $<TARGET_OBJECTS:dsp_obj>
        $<TARGET_OBJECTS:modules_obj>
        $<TARGET_OBJECTS:phasephckr_obj>
        $<TARGET_OBJECTS:juce_lib_obj>
        $<TARGET_OBJECTS:juce_phasephckr_obj>
    )

    # utilize the (modded) templates from VST SDK ...
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    include(vst_sdk)
    mod_smtg_add_vst3plugin(PhasePhckr ${VST3_SDK_ROOT} ${PHASEPHCKR_SRC})

    TARGET_LINK_LIBRARIES( PhasePhckr ${JUCE_DEPS})

    # make solution tidy
    set_target_properties(PhasePhckr PROPERTIES FOLDER targets/plugin/VST)

endif()
