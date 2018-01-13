project(phasephckr)
cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 14)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
														 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(BUILD_JUCE OFF)
option(BUILD_CLI OFF)
option(BUILD_PLUGIN_SDK OFF)

option(BUILD_GTEST OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(nlohmann_json)
include(progschj_threadpool)
if(${BUILD_GTEST})
include(gtest)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core/api)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/core)

if(${BUILD_PLUGIN_SDK})
    include_directories(${CMAKE_BINARY_DIR}/plugin_sdk_export)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/plugin_sdk)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/python)

set( PHASEPHCKR_SRC
    $<TARGET_OBJECTS:connectiongraph_obj>
    $<TARGET_OBJECTS:dsp_obj>
    $<TARGET_OBJECTS:modules_obj>
    $<TARGET_OBJECTS:phasephckr_obj>
    $<TARGET_OBJECTS:threading_obj>
)

if(${BUILD_JUCE})
    add_subdirectory(JUCE)
endif()

if(${BUILD_CLI})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cli)
endif()
