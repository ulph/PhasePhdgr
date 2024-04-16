include_directories(../source)
include_directories(../api)
include_directories(../api/phasephdgr)
include_directories(../source/connectiongraph)
include_directories(../source/modules)
include_directories(../source/dsp)
include_directories(../source/plugin)
include_directories(../source/phasephdgr/synth)
include_directories(../source/phasephdgr/design)

set( TEST_COMMON_SRC
    $<TARGET_OBJECTS:connectiongraph_obj>
    $<TARGET_OBJECTS:dsp_obj>
    $<TARGET_OBJECTS:modules_obj>
    $<TARGET_OBJECTS:phasephdgr_obj>
)

if(${SUPPORT_PLUGIN_LOADING})
    set( TEST_COMMON_SRC
        ${TEST_COMMON_SRC}
        $<TARGET_OBJECTS:plugin_obj>
    )
endif()

add_executable(bench_synthvoice bench_synthvoice.cpp ${TEST_COMMON_SRC})
set_property(TARGET bench_synthvoice PROPERTY FOLDER core/test)
target_link_libraries( bench_synthvoice ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

add_executable(run_module run_module.cpp ${TEST_COMMON_SRC})
set_property(TARGET run_module PROPERTY FOLDER core/test)
target_link_libraries( run_module ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

set(ENV{<variable>} <value>...)

if(${BUILD_PLUGIN_SDK})
    if(${SUPPORT_PLUGIN_LOADING})
        add_executable(test_plugin_loader test_plugin_loader.cpp ${TEST_COMMON_SRC})
        set_property(TARGET test_plugin_loader PROPERTY FOLDER core/test)
        target_link_libraries(test_plugin_loader ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
        add_test(NAME test_plugin_loader COMMAND test_plugin_loader WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    endif()
endif()