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

set(JUCE_EXPORT_ROOT "" CACHE FILEPATH "Folder containing exported JuceLibraryCode ...")
file(TO_CMAKE_PATH ${JUCE_EXPORT_ROOT} JUCE_EXPORT_ROOT)
include_directories(${JUCE_EXPORT_ROOT})
set(JUCE_LIB ${JUCE_EXPORT_ROOT}/JuceLibraryCode)
include_directories(${JUCE_LIB}/modules)
if(NOT EXISTS "${JUCE_EXPORT_ROOT}/JuceLibraryCode/JuceHeader.h")
    message(FATAL_ERROR "Cannot find JuceLibraryCode/JuceHeader.h (expected it in ${JUCE_EXPORT_ROOT})")
endif()

set(JUCE_SRC
    ${JUCE_LIB}/JuceHeader.h
    ${JUCE_LIB}/juce_audio_basics.cpp
)
add_library(juce_lib ${JUCE_SRC})
set_target_properties(juce_lib PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(juce_lib PROPERTIES FOLDER JUCE)
