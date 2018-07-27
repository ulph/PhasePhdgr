project(phasephckr_plugin_sdk)
cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 14)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(EX_SRC
    example.cpp
)

add_library(plugin_example.ppp SHARED ${EX_SRC})
set_target_properties(plugin_example.ppp PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(plugin_example.ppp PROPERTIES FOLDER phasephckr_plugin_sdk)

install(
    TARGETS plugin_example.ppp
    ARCHIVE DESTINATION .
    LIBRARY DESTINATION .
    RUNTIME DESTINATION .
    COMPONENT .
)
