project(phasephckr_cli)
cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 14)

find_package(Threads)


find_package(PkgConfig)

pkg_check_modules(_JACK jack)
find_library(JACK_LIBRARY NAMES jack PATHS ${_JACK_LIBDIR})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(phasephckr_cli main.cpp ${PHASEPHCKR_SRC})
set_property(TARGET phasephckr_cli PROPERTY FOLDER cli)

target_link_libraries(phasephckr_cli ${CMAKE_THREAD_LIBS_INIT} ${JACK_LIBRARY} ${CMAKE_DL_LIBS})
