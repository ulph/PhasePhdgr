set(SRC
    test_connectiongraph.cpp
)

include_directories(../source/connectiongraph)
add_executable(test_connectiongraph ${SRC})
target_link_libraries(test_connectiongraph connectiongraph)

set(TEST_MODULES_SRC
    test_modules.cpp
)

include_directories(../source/connectiongraph)
add_executable(test_modules ${TEST_MODULES_SRC})
target_link_libraries(test_modules connectiongraph)
