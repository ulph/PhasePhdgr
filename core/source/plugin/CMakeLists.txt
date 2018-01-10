set(SRC
    plugin_api.hpp
    plugin.cpp
    plugin.hpp
)

add_library(plugin_obj OBJECT ${SRC})
set_target_properties(plugin_obj PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(plugin_obj PROPERTIES FOLDER core)
