include_directories(../source)
include_directories(../api)
include_directories(../api/phasephdgr)
include_directories(../source/connectiongraph)
include_directories(../source/modules)
include_directories(../source/dsp)
include_directories(../source/plugin)
include_directories(../source/phasephdgr/synth)
include_directories(../source/phasephdgr/design)

include(CTest)

enable_testing()

set( TEST_COMMON_SRC
    $<TARGET_OBJECTS:connectiongraph_obj>
    $<TARGET_OBJECTS:dsp_obj>
    $<TARGET_OBJECTS:modules_obj>
    $<TARGET_OBJECTS:phasephdgr_obj>
)

if(${SUPPORT_PLUGIN_LOADING})
    set( TEST_COMMON_SRC
        ${TEST_COMMON_SRC}
        $<TARGET_OBJECTS:plugin_obj>
    )
endif()

add_executable(test_synthvoice test_synthvoice.cpp ${TEST_COMMON_SRC})
set_property(TARGET test_synthvoice PROPERTY FOLDER core/test)
target_link_libraries( test_synthvoice ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

add_executable(test_modules test_modules.cpp ${TEST_COMMON_SRC})
set_property(TARGET test_modules PROPERTY FOLDER core/test)
target_link_libraries( test_modules ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

if(${BUILD_PLUGIN_SDK})
    if(${SUPPORT_PLUGIN_LOADING})
        add_executable(test_plugin_sdk test_loader.cpp ${TEST_COMMON_SRC})
        set_property(TARGET test_plugin_sdk PROPERTY FOLDER core/test)
        target_link_libraries(test_plugin_sdk ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
    endif()
endif()

add_test(NAME test_synthvoice COMMAND test_synthvoice)
add_test(NAME test_modules COMMAND test_modules)
add_test(NAME test_plugin_sdk COMMAND test_plugin_sdk)
