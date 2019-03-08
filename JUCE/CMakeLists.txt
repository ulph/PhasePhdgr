project(phasephckr_juce)
cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 14)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
														 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# JUCE stuff
set(JUCE_SDK_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../submodules/JUCE)
include_directories(${JUCE_SDK_ROOT}/modules)

# VST3 SDK stuff
set(VST3_SDK_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../submodules/vst3sdk)
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

add_library(pp_common_obj OBJECT ${PH_COMMON_SRC})
set_target_properties(pp_common_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(pp_common_obj PROPERTIES FOLDER JUCE)

set(PH_SYNTH_SRC
  exportSynth.cpp
)

add_library(pp_synth_obj OBJECT ${PH_SYNTH_SRC})
set_target_properties(pp_synth_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(pp_synth_obj PROPERTIES FOLDER JUCE)

set(PH_FX_SRC
  exportFx.cpp
)

add_library(pp_fx_obj OBJECT ${PH_FX_SRC})
set_target_properties(pp_fx_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(pp_fx_obj PROPERTIES FOLDER JUCE)

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

set( PHASEPHCKR_JUCE_FX_SRC
    $<TARGET_OBJECTS:juce_fx_obj>
    $<TARGET_OBJECTS:pp_common_obj>
    $<TARGET_OBJECTS:pp_fx_obj>
)

mod_smtg_add_vst3plugin(PhasePhckr ${VST3_SDK_ROOT} ${PHASEPHCKR_SRC} ${PHASEPHCKR_JUCE_SYNTH_SRC})
TARGET_LINK_LIBRARIES(PhasePhckr ${JUCE_DEPS})
set_target_properties(PhasePhckr PROPERTIES FOLDER JUCE/VST)

mod_smtg_add_vst3plugin(PhasePhckrFx ${VST3_SDK_ROOT} ${PHASEPHCKR_SRC} ${PHASEPHCKR_JUCE_FX_SRC})
TARGET_LINK_LIBRARIES(PhasePhckrFx ${JUCE_DEPS})
set_target_properties(PhasePhckrFx PROPERTIES FOLDER JUCE/VST)

install(
    TARGETS PhasePhckr
    ARCHIVE DESTINATION .
    LIBRARY DESTINATION .
    RUNTIME DESTINATION .
    COMPONENT .
)

install(
    TARGETS PhasePhckrFx
    ARCHIVE DESTINATION .
    LIBRARY DESTINATION .
    RUNTIME DESTINATION .
    COMPONENT .
)
