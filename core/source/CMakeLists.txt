include_directories(phasephdgr connectiongraph modules dsp threading)

if(${SUPPORT_PLUGIN_LOADING})
    include_directories(plugin)
endif()

add_subdirectory(connectiongraph)
add_subdirectory(dsp)
add_subdirectory(modules)
add_subdirectory(phasephdgr)
add_subdirectory(threading)

if(${SUPPORT_PLUGIN_LOADING})
    add_subdirectory(plugin)
endif()