include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(PH_SRC
    DirectoryWatcher.cpp
    DirectoryWatcher.hpp
    GraphView.cpp
    GraphView.h
    PhasePhckrGrid.cpp
    PhasePhckrGrid.h
    PhasePhckrPluginEditor.cpp
    PhasePhckrPluginEditor.h
    PhasePhckrPluginProcessor.cpp
    PhasePhckrPluginProcessor.h
    PhasePhckrScope.cpp
    PhasePhckrScope.h
    Style.hpp
)
add_library(phasephckr_juce ${PH_SRC})
set_target_properties(phasephckr_juce PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(phasephckr_juce PROPERTIES FOLDER JUCE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules)
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules)
   message(FATAL_ERROR "Could not JUCE/modules folder at ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules. Need a symlink/folder with sdk root...")
endif()

set(JUCE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode)
if(NOT EXISTS "${JUCE_LIB}/JuceHeader.h")
    message(FATAL_ERROR "Cannot find JuceLibraryCode/JuceHeader.h ... forgot to run Projuicer at least once?")
endif()

set(JUCE_SRC
    ${JUCE_LIB}/juce_audio_basics.cpp
    ${JUCE_LIB}/juce_audio_devices.cpp
    ${JUCE_LIB}/juce_audio_formats.cpp
    ${JUCE_LIB}/juce_audio_plugin_client_AAX.cpp
    ${JUCE_LIB}/juce_audio_plugin_client_RTAS_1.cpp
    ${JUCE_LIB}/juce_audio_plugin_client_RTAS_2.cpp
    ${JUCE_LIB}/juce_audio_plugin_client_RTAS_3.cpp
    ${JUCE_LIB}/juce_audio_plugin_client_RTAS_4.cpp
    ${JUCE_LIB}/juce_audio_plugin_client_RTAS_utils.cpp
#    ${JUCE_LIB}/juce_audio_plugin_client_Standalone.cpp
    ${JUCE_LIB}/juce_audio_plugin_client_utils.cpp
    ${JUCE_LIB}/juce_audio_plugin_client_VST2.cpp
    ${JUCE_LIB}/juce_audio_plugin_client_VST3.cpp
    ${JUCE_LIB}/juce_audio_processors.cpp
    ${JUCE_LIB}/juce_core.cpp
    ${JUCE_LIB}/juce_cryptography.cpp
    ${JUCE_LIB}/juce_data_structures.cpp
    ${JUCE_LIB}/juce_events.cpp
    ${JUCE_LIB}/juce_graphics.cpp
    ${JUCE_LIB}/juce_gui_basics.cpp
    ${JUCE_LIB}/juce_gui_extra.cpp
    ${JUCE_LIB}/juce_opengl.cpp
    ${JUCE_LIB}/juce_video.cpp
    ${JUCE_LIB}/AppConfig.h
    ${JUCE_LIB}/JuceHeader.h
)
add_library(juce_lib ${JUCE_SRC})
set_target_properties(juce_lib PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(juce_lib PROPERTIES FOLDER JUCE)