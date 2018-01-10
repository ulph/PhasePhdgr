include_directories(../source)
include_directories(../api)
include_directories(../source/connectiongraph)
include_directories(../source/modules)
include_directories(../source/dsp)
include_directories(../source/plugin)
include_directories(../source/phasephckr/synth)
include_directories(../source/phasephckr/design)

set( TEST_COMMON_SRC
    $<TARGET_OBJECTS:connectiongraph_obj>
    $<TARGET_OBJECTS:dsp_obj>
    $<TARGET_OBJECTS:modules_obj>
    $<TARGET_OBJECTS:phasephckr_obj>
    $<TARGET_OBJECTS:plugin_obj>
)

add_executable(test_synthvoice test_synthvoice.cpp ${TEST_COMMON_SRC})
set_property(TARGET test_synthvoice PROPERTY FOLDER core/test)
target_link_libraries( test_synthvoice ${CMAKE_THREAD_LIBS_INIT} )

add_executable(test_modules test_modules.cpp ${TEST_COMMON_SRC})
set_property(TARGET test_modules PROPERTY FOLDER core/test)
target_link_libraries( test_modules ${CMAKE_THREAD_LIBS_INIT} )

add_executable(cli_synth cli_synth.cpp ${TEST_COMMON_SRC})
set_property(TARGET cli_synth PROPERTY FOLDER core/test)
target_link_libraries( cli_synth ${CMAKE_THREAD_LIBS_INIT} )

add_executable(test_plugin ${SRC} test_loader.cpp ${TEST_COMMON_SRC})
set_property(TARGET test_plugin PROPERTY FOLDER core/test)
target_link_libraries(test_plugin ${CMAKE_THREAD_LIBS_INIT} )