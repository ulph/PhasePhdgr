set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Threads)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../submodules/ThreadPool)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../submodules/json/src)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

option(OPT_SSE2 "build with sse2 optimisations" ON)
if(${OPT_SSE2})
    if( WIN32 )
        add_definitions(/arch:SSE2)
    else()
        add_definitions(-msse2 -mfpmath=sse)
    endif()
endif()

option(OPT_FAST_MATH "build with fast math" ON)
if(${OPT_FAST_MATH})
    if( WIN32 )
        add_definitions(/fp:fast)
    else()
        add_definitions(-ffast-math)
    endif()
endif()

if( WIN32 )
    option(DIAGNOSTICS ON)
    if(${DIAGNOSTICS})
        add_definitions(/Qvec-report:2)
    endif()
endif()

set(API_INC
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephdgr.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephdgr_json.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephdgr/components.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephdgr/design.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephdgr/docs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephdgr/examples.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephdgr/scope.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephdgr/synth.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/phasephdgr/locks.hpp
)

add_custom_target(phasephdgr_api SOURCES ${API_INC}) # hack
set_target_properties(phasephdgr_api PROPERTIES FOLDER core)

include_directories(api)
add_subdirectory(source)

enable_testing()
add_subdirectory(test)
