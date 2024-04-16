configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sdk/example.cpp ${CMAKE_BINARY_DIR}/plugin_sdk_export/example.cpp COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sdk/CMakeLists.txt ${CMAKE_BINARY_DIR}/plugin_sdk_export/CMakeLists.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../core/source/plugin/plugin_api.hpp ${CMAKE_BINARY_DIR}/plugin_sdk_export/plugin_api.hpp COPYONLY)

include_directories(${CMAKE_BINARY_DIR}/plugin_sdk_export)

add_subdirectory(${CMAKE_BINARY_DIR}/plugin_sdk_export ${CMAKE_BINARY_DIR}/plugin_sdk_build) # hax
