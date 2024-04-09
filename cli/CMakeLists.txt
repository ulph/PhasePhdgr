find_package(Threads)


find_package(PkgConfig)

pkg_check_modules(_JACK jack)
find_library(JACK_LIBRARY NAMES jack PATHS ${_JACK_LIBDIR})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(phasephdgr_cli main.cpp ${PHASEPHDGR_SRC})
set_property(TARGET phasephdgr_cli PROPERTY FOLDER cli)

target_link_libraries(phasephdgr_cli ${CMAKE_THREAD_LIBS_INIT} ${JACK_LIBRARY} ${CMAKE_DL_LIBS})
