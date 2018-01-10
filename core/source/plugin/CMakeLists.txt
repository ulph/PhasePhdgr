set(SRC
    plugin.cpp
    plugin.hpp
)

add_library(plugin_obj OBJECT ${SRC})
set_target_properties(plugin_obj PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(plugin_obj PROPERTIES FOLDER core/plugin)

set(EX_SRC
    example.cpp
)

add_library(plugin_example SHARED ${EX_SRC})
set_target_properties(plugin_example PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(plugin_example PROPERTIES FOLDER core/plugin)

set(TEST_SRC
    test_loader.cpp
)

add_executable(plugin_test ${SRC} ${TEST_SRC})
set_target_properties(plugin_test PROPERTIES FOLDER core/plugin)