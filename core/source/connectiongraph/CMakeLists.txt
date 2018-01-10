set(SRC
    connectiongraph.cpp
    connectiongraph.hpp
    module.cpp
    module.hpp
    instruction.hpp
)

add_library(connectiongraph_obj OBJECT ${SRC})
set_target_properties(connectiongraph_obj PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(connectiongraph_obj PROPERTIES FOLDER core)

add_library(module_sdk_obj OBJECT module.cpp module.hpp)
set_target_properties(module_sdk_obj PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(module_sdk_obj PROPERTIES FOLDER core/plugin)
