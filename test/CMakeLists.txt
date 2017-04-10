include_directories(../source/connectiongraph)
add_executable(test_connectiongraph test_connectiongraph.cpp)
target_link_libraries(test_connectiongraph connectiongraph)

include_directories(../source)
include_directories(../include)
add_executable(test_performance test_performance.cpp)
target_link_libraries(test_performance phasephckr connectiongraph)

set(TEST_MODULES_SRC
    test_modules.cpp
)

include_directories(../source/connectiongraph)
add_executable(test_modules ${TEST_MODULES_SRC})
target_link_libraries(test_modules connectiongraph)
