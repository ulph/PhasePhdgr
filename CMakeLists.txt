project(phasephckr)
cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)

find_package(Threads)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories(${CMAKE_SOURCE_DIR}/include)

option(USING_NLOHMANN_JSON OFF)
if(${USING_NLOHMANN_JSON})
    set(JSON_INCLUDE_DIR "" CACHE FILEPATH "Install dir of https://nlohmann.github.io/json/")
    include_directories(${JSON_INCLUDE_DIR})
    if(NOT EXISTS "${JSON_INCLUDE_DIR}/nlohmann/json.hpp")
        message(FATAL_ERROR "Cannot find nlohmann/json.hpp (expected it in ${JSON_INCLUDE_DIR})")
    endif()
    add_definitions(-DUSING_NLOHMANN_JSON=1)
    install(FILES "${JSON_INCLUDE_DIR}/nlohmann/json.hpp" DESTINATION include/nlohmann)
else()
    add_definitions(-DUSING_NLOHMANN_JSON=0)
endif()

add_subdirectory(source)
add_subdirectory(test)