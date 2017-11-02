project(phasephckr_core)
cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 14)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Threads)

set(JSON_INCLUDE_DIR "" CACHE FILEPATH "Install dir of https://nlohmann.github.io/json/")
  include_directories(${JSON_INCLUDE_DIR})
  if(NOT EXISTS "${JSON_INCLUDE_DIR}/nlohmann/json.hpp")
    message(FATAL_ERROR "Cannot find nlohmann/json.hpp")
endif()

add_definitions(-msse2 -mfpmath=sse)
option(FAST_MATH OFF)
if(${FAST_MATH})
    add_definitions(-ffast-math)
else()
endif()

set(API_INC
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr_json.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr/components.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr/docs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr/design.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr/synth.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephckr/scope.hpp
)
add_custom_target(phasephckr_api SOURCES ${API_INC}) # hack

include_directories(api)
add_subdirectory(source)

option(BUILD_TESTS OFF)
if(${BUILD_TESTS})
    add_subdirectory(test)
endif()
