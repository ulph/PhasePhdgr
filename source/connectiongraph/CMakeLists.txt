set(SRC
    connectiongraph.cpp
    connectiongraph.hpp
    module.hpp
    instruction.hpp
)

add_library(connectiongraph_obj OBJECT ${SRC})
set_property(TARGET connectiongraph_obj PROPERTY FOLDER obj)

add_library(connectiongraph ${SRC})

set_target_properties(connectiongraph PROPERTIES COMPILE_FLAGS "-fPIC")

install(TARGETS connectiongraph DESTINATION lib)
