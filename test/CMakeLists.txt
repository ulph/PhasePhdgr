set(SRC
    test_connectiongraph.cpp
)

include_directories(../source/connectiongraph)
add_executable(test_connectiongraph ${SRC})
target_link_libraries(test_connectiongraph connectiongraph)
