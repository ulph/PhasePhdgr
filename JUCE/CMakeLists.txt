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
set_target_properties(juce_phasephckr_obj PROPERTIES FOLDER JUCE)
