set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Threads)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../submodules/ThreadPool)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../submodules/json/src)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

function(add_definition_if_supported option)
    include(CheckCXXCompilerFlag)
    # Sanitize the option name for use in variable naming
    string(REPLACE "-" "_" OPTION_VAR ${option})
    string(REPLACE "=" "_" OPTION_VAR ${OPTION_VAR})
    set(FLAG_CHECK_VAR "COMPILER_SUPPORTS_${OPTION_VAR}")
    # Check if this was already tested
    if(NOT DEFINED ${FLAG_CHECK_VAR})
        check_cxx_compiler_flag("${option}" ${FLAG_CHECK_VAR})
    endif()
    # Apply the option if supported
    if(${FLAG_CHECK_VAR})
        add_definitions(${option})
    endif()
endfunction()

option(OPT_SSE2 "build with sse2 optimisations" ON)
if(${OPT_SSE2})
    if( WIN32 )
        add_definitions(/arch:SSE2)
    else()
        add_definition_if_supported(-msse2)
        add_definition_if_supported(-mfpmath=sse)
    endif()
endif()

option(OPT_FAST_MATH "build with fast math" ON)
if(${OPT_FAST_MATH})
    if( WIN32 )
        add_definitions(/fp:fast)
    else()
        add_definition_if_supported(-ffast-math)
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

add_subdirectory(test)
