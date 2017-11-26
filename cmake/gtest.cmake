cmake_minimum_required(VERSION 2.8.2)

set(GTEST_DIR "" CACHE FILEPATH "Root directory of GTEST")

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${GTEST_DIR} ${CMAKE_BINARY_DIR}/googletest-build)