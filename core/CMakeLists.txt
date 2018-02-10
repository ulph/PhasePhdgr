project(phasephckr_core)
cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 14)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Threads)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../submodules/ThreadPool)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../submodules/json/src)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

option(OPT_SSE2 ON)
if(${OPT_SSE2})
    add_definitions(-msse2 -mfpmath=sse)
    add_definitions(/arch:SSE2)
endif()

option(OPT_FAST_MATH ON)
if(${OPT_FAST_MATH})
    add_definitions(-ffast-math)
    add_definitions(/fp:fast)
endif()

option(DIAGNOSTICS ON)
if(${DIAGNOSTICS})
    add_definitions(/Qvec-report:2)
endif()

set(API_INC
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr_json.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr/components.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr/design.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr/docs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr/examples.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr/scope.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr/synth.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr/locks.hpp
)

add_custom_target(phasephckr_api SOURCES ${API_INC}) # hack
set_target_properties(phasephckr_api PROPERTIES FOLDER core)

include_directories(api)
add_subdirectory(source)

option(BUILD_TESTS OFF)
if(${BUILD_TESTS})
    add_subdirectory(test)
endif()
