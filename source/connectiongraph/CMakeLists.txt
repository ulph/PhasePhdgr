set(SRC
    connectiongraph.cpp
    connectiongraph.hpp
    module.hpp
    instruction.hpp
)

add_library(connectiongraph_obj OBJECT ${SRC})
add_library(connectiongraph $<TARGET_OBJECTS:connectiongraph_obj>)
set_property(TARGET connectiongraph PROPERTY FOLDER targets/lib)
set_target_properties(connectiongraph PROPERTIES COMPILE_FLAGS "-fPIC")

install(TARGETS connectiongraph DESTINATION lib)
