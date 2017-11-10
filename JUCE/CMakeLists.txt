project(phasephckr_juce)
cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 14)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
														 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories(${JUCE_SDK_ROOT}/modules)

add_subdirectory(JuceJunk)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/JuceJunk/JuceLibraryCode)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/PatchEditor)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/FileBrowser)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Components)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Utils)

set(PH_SRC
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
    Components/PhasePhckrGrid.cpp
    Components/PhasePhckrGrid.h
    Components/PhasePhckrScope.cpp
    Components/PhasePhckrScope.h
    Components/ParameterKnob.hpp

    Utils/Utils.hpp
    Utils/Style.cpp
    Utils/Style.hpp

    PhasePhckrParameter.cpp
    PhasePhckrParameter.hpp

    PhasePhckrPluginEditor.cpp
    PhasePhckrPluginEditor.h
    PhasePhckrPluginProcessor.cpp
    PhasePhckrPluginProcessor.h
)

add_library(juce_phasephckr_obj OBJECT ${PH_SRC})
set_target_properties(juce_phasephckr_obj PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(juce_phasephckr_obj PROPERTIES FOLDER JUCE)


option(INTERCEPT_STD_STREAMS ON)
option(FORCE_FTZ_DAZ OFF)


if(${INTERCEPT_STD_STREAMS})
    add_definitions(-DINTERCEPT_STD_STREAMS=1)
else()
    add_definitions(-DINTERCEPT_STD_STREAMS=0)
endif()

if(${FORCE_FTZ_DAZ})
    add_definitions(-DFORCE_FTZ_DAZ=1)
else()
    add_definitions(-DFORCE_FTZ_DAZ=0)
endif()


# VST3 SDK stuff
set(VST3_SDK_ROOT "" CACHE FILEPATH "VST3 SDK root")
include_directories(${VST3_SDK_ROOT})

# JUCE stuff
set(JUCE_SDK_ROOT "" CACHE FILEPATH "JUCE5 SDK root")

# our + JUCE object libs
set( PHASEPHCKR_JUCE_SRC
    $<TARGET_OBJECTS:juce_lib_obj>
    $<TARGET_OBJECTS:juce_phasephckr_obj>
)

# utilize the (modded) templates from VST SDK ...
include(vst_sdk)
mod_smtg_add_vst3plugin(PhasePhckr ${VST3_SDK_ROOT} ${PHASEPHCKR_SRC} ${PHASEPHCKR_JUCE_SRC})

TARGET_LINK_LIBRARIES( PhasePhckr ${JUCE_DEPS})

# make solution tidy
set_target_properties(PhasePhckr PROPERTIES FOLDER JUCE/VST)
