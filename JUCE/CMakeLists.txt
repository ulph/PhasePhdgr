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
    ${JUCE_LIB}/JuceHeader.h
    ${JUCE_LIB}/juce_audio_basics.cpp
)
add_library(juce_lib ${JUCE_SRC})
set_target_properties(juce_lib PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(juce_lib PROPERTIES FOLDER JUCE)
