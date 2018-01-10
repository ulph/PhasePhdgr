set(SRC
    plugin_api.hpp
    plugin.cpp
    plugin.hpp
)

add_library(plugin_obj OBJECT ${SRC})
set_target_properties(plugin_obj PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(plugin_obj PROPERTIES FOLDER core)

set(EX_SRC
    example.cpp
)

add_library(plugin_example SHARED ${EX_SRC} $<TARGET_OBJECTS:module_sdk_obj>)
set_target_properties(plugin_example PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(plugin_example PROPERTIES FOLDER core/plugin)
