include_directories(../source/connectiongraph)
include_directories(../source/modules)
add_executable(test_connectiongraph test_connectiongraph.cpp)
target_link_libraries(test_connectiongraph connectiongraph modules)

include_directories(../source)
include_directories(../include)
add_executable(test_performance test_performance.cpp)
target_link_libraries(test_performance phasephckr connectiongraph modules)

set(TEST_MODULES_SRC
    test_modules.cpp
)

include_directories(../source/connectiongraph)
add_executable(test_modules ${TEST_MODULES_SRC})
target_link_libraries(test_modules connectiongraph)


add_executable(test_synth test_synth.cpp)
target_link_libraries(test_synth phasephckr connectiongraph)
