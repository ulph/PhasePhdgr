include_directories(../source/connectiongraph)
include_directories(../source/modules)
add_executable(test_connectiongraph test_connectiongraph.cpp)
target_link_libraries(test_connectiongraph connectiongraph modules dsp)
set_property(TARGET test_connectiongraph PROPERTY FOLDER targets/test)

include_directories(../source)
include_directories(../include)
add_executable(test_performance test_performance.cpp)
target_link_libraries(test_performance phasephckr connectiongraph modules dsp)
set_property(TARGET test_performance PROPERTY FOLDER targets/test)

include_directories(../source/connectiongraph)
add_executable(test_modules test_modules.cpp)
target_link_libraries(test_modules connectiongraph modules dsp)
set_property(TARGET test_modules PROPERTY FOLDER targets/test)

add_executable(test_biquad test_biquad.cpp)
target_link_libraries(test_biquad connectiongraph modules dsp)
set_property(TARGET test_biquad PROPERTY FOLDER targets/test)

add_executable(test_synth test_synth.cpp)
target_link_libraries(test_synth phasephckr connectiongraph modules dsp)
set_property(TARGET test_synth PROPERTY FOLDER targets/test)

add_executable(test_camelenvelope test_camelenvelope.cpp)
target_link_libraries(test_camelenvelope phasephckr connectiongraph modules dsp)
set_property(TARGET test_camelenvelope PROPERTY FOLDER targets/test)

if(${USING_NLOHMANN_JSON})
add_executable(test_serialization test_serialization.cpp)
target_link_libraries(test_serialization phasephckr connectiongraph modules dsp)
set_property(TARGET test_serialization PROPERTY FOLDER targets/test)
endif()
