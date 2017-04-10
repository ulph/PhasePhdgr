set(SRC
    connectiongraph.cpp
    connectiongraph.hpp
    module.hpp
    inputoutputbus.hpp
)

add_library(connectiongraph ${SRC})

set_target_properties(connectiongraph PROPERTIES COMPILE_FLAGS "-fPIC")

install(TARGETS connectiongraph DESTINATION lib)
