set(SRC
    connectiongraph.cpp
    connectiongraph.hpp
    module.hpp
    moduleaccessor.hpp
    instruction.hpp
)

add_library(connectiongraph_obj OBJECT ${SRC})
set_target_properties(connectiongraph_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(connectiongraph_obj PROPERTIES FOLDER core)

configure_file(module.hpp ${CMAKE_BINARY_DIR}/plugin_sdk_export/module.hpp COPYONLY)
