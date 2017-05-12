include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include(FindFreetype)
include_directories(${FREETYPE_INCLUDE_DIRS})

set(PH_SRC
    DirectoryWatcher.cpp
    DirectoryWatcher.hpp
    GraphEditor.cpp
    GraphEditor.hpp
    GraphEditorPositions.cpp
    GraphEditorPositions.hpp
    GraphEditorModel.hpp
    GraphEditorModel.cpp
    PatchEditor.hpp
    PatchEditor.cpp
    PhasePhckrGrid.cpp
    PhasePhckrGrid.h
    PhasePhckrPluginEditor.cpp
    PhasePhckrPluginEditor.h
    PhasePhckrPluginProcessor.cpp
    PhasePhckrPluginProcessor.h
    PhasePhckrScope.cpp
    PhasePhckrScope.h
    Utils.hpp
)

add_library(juce_phasephckr_obj OBJECT ${PH_SRC})
set_target_properties(juce_phasephckr_obj PROPERTIES COMPILE_FLAGS "-fPIC")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules)
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules)
   message(FATAL_ERROR "Could not JUCE/modules folder at ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules. Need a symlink/folder with sdk root...")
endif()

set(JUCE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode)
if(NOT EXISTS "${JUCE_LIB}/JuceHeader.h")
    message(FATAL_ERROR "Cannot find ${JUCE_LIB}/JuceHeader.h ... forgot to run Projuicer at least once?")
endif()

set(JUCE_SRC
    ${JUCE_LIB}/include_juce_audio_basics.cpp
#    ${JUCE_LIB}/include_juce_audio_devices.cpp
#    ${JUCE_LIB}/include_juce_audio_formats.cpp
#    ${JUCE_LIB}/include_juce_audio_plugin_client_AAX.cpp
#    ${JUCE_LIB}/include_juce_audio_plugin_client_RTAS_1.cpp
#    ${JUCE_LIB}/include_juce_audio_plugin_client_RTAS_2.cpp
#    ${JUCE_LIB}/include_juce_audio_plugin_client_RTAS_3.cpp
#    ${JUCE_LIB}/include_juce_audio_plugin_client_RTAS_4.cpp
#    ${JUCE_LIB}/include_juce_audio_plugin_client_RTAS_utils.cpp
#    ${JUCE_LIB}/include_juce_audio_plugin_client_Standalone.cpp
    ${JUCE_LIB}/include_juce_audio_plugin_client_utils.cpp
#    ${JUCE_LIB}/include_juce_audio_plugin_client_VST2.cpp
    ${JUCE_LIB}/include_juce_audio_plugin_client_VST3.cpp
    ${JUCE_LIB}/include_juce_audio_processors.cpp
    ${JUCE_LIB}/include_juce_core.cpp
#    ${JUCE_LIB}/include_juce_cryptography.cpp
    ${JUCE_LIB}/include_juce_data_structures.cpp
    ${JUCE_LIB}/include_juce_events.cpp
    ${JUCE_LIB}/include_juce_graphics.cpp
    ${JUCE_LIB}/include_juce_gui_basics.cpp
#    ${JUCE_LIB}/include_juce_gui_extra.cpp
#    ${JUCE_LIB}/include_juce_opengl.cpp
#    ${JUCE_LIB}/include_juce_video.cpp
    ${JUCE_LIB}/AppConfig.h
    ${JUCE_LIB}/JuceHeader.h
)

if(APPLE)
    add_definitions(-x objective-c++)
    set(JUCE_SRC
        ${JUCE_SRC}
        ${JUCE_LIB}/include_juce_opengl.mm
        ${JUCE_LIB}/include_juce_gui_extra.mm
    )
endif()


add_library(juce_lib_obj OBJECT ${JUCE_SRC})
set_target_properties(juce_lib_obj PROPERTIES COMPILE_FLAGS "-fPIC")
