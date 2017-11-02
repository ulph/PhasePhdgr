include_directories(../source)
include_directories(../api)
include_directories(../source/connectiongraph)
include_directories(../source/modules)
include_directories(../source/dsp)
include_directories(../source/phasephckr/synth)
include_directories(../source/phasephckr/design)

set( TEST_COMMON_SRC
    $<TARGET_OBJECTS:connectiongraph_obj>
    $<TARGET_OBJECTS:dsp_obj>
    $<TARGET_OBJECTS:modules_obj>
    $<TARGET_OBJECTS:phasephckr_obj>
)

add_executable(test_synthvoice test_synthvoice.cpp ${TEST_COMMON_SRC})
set_property(TARGET test_synthvoice PROPERTY FOLDER targets/test)

add_executable(test_modules test_modules.cpp ${TEST_COMMON_SRC})
set_property(TARGET test_modules PROPERTY FOLDER targets/test)

add_executable(cli_synth cli_synth.cpp ${TEST_COMMON_SRC})
set_property(TARGET cli_synth PROPERTY FOLDER targets/test)
