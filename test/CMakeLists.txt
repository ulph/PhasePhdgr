include_directories(../source)
include_directories(../include)
include_directories(../source/connectiongraph)
include_directories(../source/modules)
include_directories(../source/dsp)

set( TEST_COMMON_SRC
    $<TARGET_OBJECTS:connectiongraph_obj>
    $<TARGET_OBJECTS:dsp_obj>
    $<TARGET_OBJECTS:modules_obj>
    $<TARGET_OBJECTS:phasephckr_obj>
)

add_executable(test_performance test_performance.cpp ${TEST_COMMON_SRC})
set_property(TARGET test_performance PROPERTY FOLDER targets/test)

add_executable(test_modules test_modules.cpp ${TEST_COMMON_SRC})
set_property(TARGET test_modules PROPERTY FOLDER targets/test)

add_executable(test_synth test_synth.cpp ${TEST_COMMON_SRC})
set_property(TARGET test_synth PROPERTY FOLDER targets/test)
