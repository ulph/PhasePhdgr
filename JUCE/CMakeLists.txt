project(phasephckr_juce)
cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 14)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
														 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# JUCE stuff
set(JUCE_SDK_ROOT "" CACHE FILEPATH "JUCE5 SDK root")
include_directories(${JUCE_SDK_ROOT}/modules)

# VST3 SDK stuff
set(VST3_SDK_ROOT "" CACHE FILEPATH "VST3 SDK root")
include_directories(${VST3_SDK_ROOT})

add_subdirectory(JuceJunk)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/JuceJunk/JuceLibraryCode)

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
    Utils/Style.cpp
    Utils/Style.hpp

    Parameters.cpp
    Parameters.hpp

    Parameter.hpp

    PluginCommon.h
    PluginCommon.cpp

    SettingsEditor.hpp
    SettingsEditor.cpp

)

add_library(pp_common_obj OBJECT ${PH_COMMON_SRC})
set_target_properties(pp_common_obj PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(pp_common_obj PROPERTIES FOLDER JUCE)

set(PH_SYNTH_SRC
  FileBrowser/FileBrowserPanel.cpp
  FileBrowser/FileBrowserPanel.hpp
  PluginEditor.cpp
  PluginEditor.h
  PluginProcessor.cpp
  PluginProcessor.h
  exportSynth.cpp
)

add_library(pp_synth_obj OBJECT ${PH_SYNTH_SRC})
set_target_properties(pp_synth_obj PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(pp_synth_obj PROPERTIES FOLDER JUCE)

set(PH_EFFECT_SRC
  FileBrowser/FileBrowserPanelFX.cpp
  FileBrowser/FileBrowserPanelFX.hpp
  PluginEditorFX.cpp
  PluginEditorFX.h
  PluginProcessorFX.cpp
  PluginProcessorFX.h
  exportEffect.cpp
)

add_library(pp_effect_obj OBJECT ${PH_EFFECT_SRC})
set_target_properties(pp_effect_obj PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(pp_effect_obj PROPERTIES FOLDER JUCE)

option(INTERCEPT_STD_STREAMS ON)

if(${INTERCEPT_STD_STREAMS})
    add_definitions(-DINTERCEPT_STD_STREAMS=1)
else()
    add_definitions(-DINTERCEPT_STD_STREAMS=0)
endif()

# utilize the (modded) templates from VST SDK ...
include(vst_sdk)

set( PHASEPHCKR_JUCE_SYNTH_SRC
    $<TARGET_OBJECTS:juce_synth_obj>
    $<TARGET_OBJECTS:pp_common_obj>
    $<TARGET_OBJECTS:pp_synth_obj>
)

mod_smtg_add_vst3plugin(PhasePhckr ${VST3_SDK_ROOT} ${PHASEPHCKR_SRC} ${PHASEPHCKR_JUCE_SYNTH_SRC})
TARGET_LINK_LIBRARIES( PhasePhckr ${JUCE_DEPS})
set_target_properties(PhasePhckr PROPERTIES FOLDER JUCE/VST)


set( PHASEPHCKR_JUCE_EFFECT_SRC
    $<TARGET_OBJECTS:juce_effect_obj>
    $<TARGET_OBJECTS:pp_common_obj>
    $<TARGET_OBJECTS:pp_effect_obj>
)

mod_smtg_add_vst3plugin(PhasePhckrFX ${VST3_SDK_ROOT} ${PHASEPHCKR_SRC} ${PHASEPHCKR_JUCE_EFFECT_SRC})
TARGET_LINK_LIBRARIES( PhasePhckrFX ${JUCE_DEPS})
set_target_properties(PhasePhckrFX PROPERTIES FOLDER JUCE/VST)
