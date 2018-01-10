set(EX_SRC
    example.cpp
)

add_library(plugin_example SHARED ${EX_SRC})
set_target_properties(plugin_example PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(plugin_example PROPERTIES FOLDER plugin_sdk)
