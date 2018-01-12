include_directories(phasephckr connectiongraph modules dsp threading)

add_subdirectory(connectiongraph)
add_subdirectory(dsp)
add_subdirectory(modules)
add_subdirectory(phasephckr)
add_subdirectory(threading)

if(${BUILD_PLUGIN_SDK})
  add_subdirectory(plugin)
endif()