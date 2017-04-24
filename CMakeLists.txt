project(phasephckr)
cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(include)

find_package(Threads)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(USING_NLOHMANN_JSON OFF)
if(${USING_NLOHMANN_JSON})
    set(JSON_INCLUDE_DIR "" CACHE FILEPATH "Install dir of https://nlohmann.github.io/json/")
    include_directories(${JSON_INCLUDE_DIR})
    if(NOT EXISTS "${JSON_INCLUDE_DIR}/nlohmann/json.hpp")
        message(FATAL_ERROR "Cannot find nlohmann/json.hpp")
    endif()
    add_definitions(-DUSING_NLOHMANN_JSON=1)
    install(FILES "${JSON_INCLUDE_DIR}/nlohmann/json.hpp" DESTINATION include/nlohmann)
else()
    add_definitions(-DUSING_NLOHMANN_JSON=0)
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

install(FILES 
    ${API_INC}
    DESTINATION include
)

option(BUILD_JUCE_STUFF OFF)
if(${BUILD_JUCE_STUFF})
    add_subdirectory(JUCE)
endif()

option(BUILD_VST OFF)
if(${BUILD_VST})
    set(VST_DESTINATION "~/.vst" CACHE FILEPATH "Where to INSTALL the VST")
    add_library(
        PhasePhckr SHARED
        $<TARGET_OBJECTS:connectiongraph_obj>
        $<TARGET_OBJECTS:dsp_obj>
        $<TARGET_OBJECTS:modules_obj>
        $<TARGET_OBJECTS:phasephckr_obj>
        $<TARGET_OBJECTS:juce_lib_obj>
        $<TARGET_OBJECTS:juce_phasephckr_obj>
    )
    set_target_properties(PhasePhckr PROPERTIES FOLDER targets/plugin/VST)
    install(TARGETS PhasePhckr DESTINATION ${VST_DESTINATION})
endif()
