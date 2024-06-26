set_property(GLOBAL PROPERTY USE_FOLDERS ON)
														 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_definitions(-DNDEBUG)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/PatchEditor)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/FileBrowser)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Components)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Utils)

set(PH_COMMON_SRC
    PatchEditor/GraphEditor.cpp
    PatchEditor/GraphEditor.hpp
    PatchEditor/GraphEditorPositions.cpp
    PatchEditor/GraphEditorPositions.hpp
    PatchEditor/GraphEditorModel.hpp
    PatchEditor/GraphEditorModel.cpp
    PatchEditor/PatchEditor.hpp
    PatchEditor/PatchEditor.cpp
    PatchEditor/PopupMenus.hpp
    PatchEditor/CommonEditor.hpp
    PatchEditor/CommonEditor.cpp

    FileBrowser/DirectoryWatcher.cpp
    FileBrowser/DirectoryWatcher.hpp
    FileBrowser/FileEditorBundle.cpp
    FileBrowser/FileEditorBundle.hpp
    FileBrowser/FileIO.hpp
    FileBrowser/FileIO.cpp

    Components/DocView.cpp
    Components/DocView.hpp
    Components/PPGrid.cpp
    Components/PPGrid.h
    Components/ScopeView.cpp
    Components/ScopeView.h
    Components/ParameterKnob.hpp

    Utils/Utils.hpp

    Parameters.cpp
    Parameters.hpp

    Parameter.hpp

    PluginCommon.h
    PluginCommon.cpp

    SettingsEditor.hpp
    SettingsEditor.cpp

    FileBrowser/FileBrowserPanel.cpp
    FileBrowser/FileBrowserPanel.hpp
    PluginEditor.cpp
    PluginEditor.h
    PluginProcessor.cpp
    PluginProcessor.h
)

option(INTERCEPT_STD_STREAMS ON)

if(${INTERCEPT_STD_STREAMS})
    add_definitions(-DINTERCEPT_STD_STREAMS=1)
else()
    add_definitions(-DINTERCEPT_STD_STREAMS=0)
endif()

set( USED_JUCE_LIBS
    juce::juce_audio_utils
    juce::juce_events
)

set( USED_JUCE_FLAGS
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
#    juce::juce_recommended_warning_flags
)

set( USED_JUCE_COMPILE_DEFINITIONS
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_MODAL_LOOPS_PERMITTED=1 # workaround, should really redo the pop up menus... see https://forum.juce.com/t/6-1-issue-with-show-is-not-a-member-of-juce-popupmenu/47511
)

juce_add_plugin(PhasePhdgr
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PLUGIN_MANUFACTURER_CODE "Ulha"
    PLUGIN_CODE Ulh0
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "PhasePhdgr"
)
target_sources(PhasePhdgr PRIVATE exportSynth.cpp ${PH_COMMON_SRC} ${PHASEPHDGR_SRC})
target_link_libraries(PhasePhdgr PRIVATE ${USED_JUCE_LIBS} PUBLIC ${USED_JUCE_FLAGS})
target_compile_definitions(PhasePhdgr PUBLIC ${USED_JUCE_COMPILE_DEFINITIONS})
set_target_properties(PhasePhdgr PROPERTIES FOLDER JUCE/VST)

juce_add_plugin(PhasePhdgrFx
    IS_SYNTH FALSE
    PLUGIN_MANUFACTURER_CODE "Ulha"
    PLUGIN_CODE Ulh1
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "PhasePhdgrFx"
)
target_sources(PhasePhdgrFx PRIVATE exportFx.cpp ${PH_COMMON_SRC} ${PHASEPHDGR_SRC})
target_link_libraries(PhasePhdgrFx PRIVATE ${USED_JUCE_LIBS} PUBLIC ${USED_JUCE_FLAGS})
target_compile_definitions(PhasePhdgrFx PUBLIC ${USED_JUCE_COMPILE_DEFINITIONS})
set_target_properties(PhasePhdgrFx PROPERTIES FOLDER JUCE/VST)