project(phasephckr)
cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(include)

find_package(Threads)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_definitions(-msse2 -mfpmath=sse)
option(FAST_MATH OFF)
if(${FAST_MATH})
    add_definitions(-ffast-math)
else()
endif()

option(BUILD_VST OFF)
if(${BUILD_VST})
    set(USING_NLOHMANN_JSON ON)
    set(JSON_INCLUDE_DIR "" CACHE FILEPATH "Install dir of https://nlohmann.github.io/json/")
    include_directories(${JSON_INCLUDE_DIR})
    if(NOT EXISTS "${JSON_INCLUDE_DIR}/nlohmann/json.hpp")
        message(FATAL_ERROR "Cannot find nlohmann/json.hpp")
    endif()
else()
    set(USING_NLOHMANN_JSON OFF)
endif()

add_subdirectory(source)

option(BUILD_TESTS OFF)
if(${BUILD_TESTS})
    add_subdirectory(test)
endif()

set(API_INC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/phasephckr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/components.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/docs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/design.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/design_json.hpp
)

add_custom_target(phasephckr_api SOURCES ${API_INC}) # hack

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
    include_directories(${VST3_SDK_ROOT}) #JUCE needs this

    # JUCE stuff
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

    # extra junk required by JUCE on APPLE
    if(APPLE)
        FIND_LIBRARY(COREFOUNDATION_LIB CoreFoundation)
        TARGET_LINK_LIBRARIES( PhasePhckr PRIVATE ${COREFOUNDATION_LIB} )
    endif()

    TARGET_LINK_LIBRARIES( PhasePhckr PRIVATE ${JUCE_DEPS})

    # make solution tidy
    set_target_properties(PhasePhckr PROPERTIES FOLDER targets/plugin/VST)

endif()
