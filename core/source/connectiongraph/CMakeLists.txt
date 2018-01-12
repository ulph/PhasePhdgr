set(SRC
    connectiongraph.cpp
    connectiongraph.hpp
    module.hpp
    moduleaccessor.hpp
    instruction.hpp
)

add_library(connectiongraph_obj OBJECT ${SRC})
set_target_properties(connectiongraph_obj PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(connectiongraph_obj PROPERTIES FOLDER core)
