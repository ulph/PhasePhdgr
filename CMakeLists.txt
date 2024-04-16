cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.5)
project(phasephdgr VERSION 0.1.1)

set (CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
														 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_BUILD_RPATH ${CMAKE_BINARY_DIR})

option(BUILD_JUCE "Build JUCE based artifacts (VST, standalone apps)" OFF)
option(BUILD_CLI "Build some CLIs" OFF)
option(SUPPORT_PLUGIN_LOADING "Build in support for loading (sub)plugins" ON)
option(BUILD_PLUGIN_SDK "Export the (sub)plugin SDK" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/submodules/ThreadPool)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/submodules/json/src)

if(${SUPPORT_PLUGIN_LOADING})
    add_definitions(-DSUPPORT_PLUGIN_LOADING=1)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core/api)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/core)

if(${BUILD_PLUGIN_SDK})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/plugin_sdk)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/python)

set( PHASEPHDGR_SRC
    $<TARGET_OBJECTS:connectiongraph_obj>
    $<TARGET_OBJECTS:dsp_obj>
    $<TARGET_OBJECTS:modules_obj>
    $<TARGET_OBJECTS:phasephdgr_obj>
    $<TARGET_OBJECTS:threading_obj>
)

if(${SUPPORT_PLUGIN_LOADING})
    set( PHASEPHDGR_SRC
        ${PHASEPHDGR_SRC}
        $<TARGET_OBJECTS:plugin_obj>
    )
endif()

if(${BUILD_JUCE})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/submodules/JUCE)
  add_subdirectory(JUCE)
endif()

if(${BUILD_CLI})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cli)
endif()
