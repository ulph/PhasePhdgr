set(SRC
    plugin_api.hpp
    pluginloader.cpp
    pluginloader.hpp
    pluginsregister.cpp
    pluginsregister.hpp
)

add_library(plugin_obj OBJECT ${SRC})
set_target_properties(plugin_obj PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(plugin_obj PROPERTIES FOLDER core)

configure_file(plugin_api.hpp ${CMAKE_BINARY_DIR}/plugin_sdk_export/plugin_api.hpp COPYONLY)
