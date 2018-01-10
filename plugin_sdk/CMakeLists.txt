set(EX_SRC
    example.cpp
)

add_library(plugin_example.ppp SHARED ${EX_SRC})
set_target_properties(plugin_example.ppp PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(plugin_example.ppp PROPERTIES FOLDER plugin_sdk)
