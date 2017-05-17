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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules)
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules)
   message(FATAL_ERROR "Could not JUCE/modules folder at ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules. Need a symlink/folder with sdk root...")
endif()

set(JUCE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode)
if(NOT EXISTS "${JUCE_LIB}/JuceHeader.h")
    message(FATAL_ERROR "Cannot find ${JUCE_LIB}/JuceHeader.h ... forgot to run Projuicer at least once?")
endif()

#add_definitions(-DJucePlugin_Build_VST=1)
add_definitions(-DJucePlugin_Build_VST3=1)
add_definitions(-DJucePlugin_WantsMidiInput=1)
add_definitions(-DJucePlugin_IsSynth=1)

set(JUCE_SRC
    ${JUCE_LIB}/include_juce_audio_plugin_client_utils.cpp
#    ${JUCE_LIB}/include_juce_audio_plugin_client_VST2.cpp
    ${JUCE_LIB}/include_juce_audio_plugin_client_VST3.cpp
    ${JUCE_LIB}/AppConfig.h
    ${JUCE_LIB}/JuceHeader.h
)

if(APPLE)
    add_definitions(-x objective-c++)
    set(JUCE_SRC
        ${JUCE_SRC}
        ${JUCE_LIB}/include_juce_data_structures.mm
        ${JUCE_LIB}/include_juce_events.mm
        ${JUCE_LIB}/include_juce_audio_basics.mm
        ${JUCE_LIB}/include_juce_core.mm
        ${JUCE_LIB}/include_juce_opengl.mm
        ${JUCE_LIB}/include_juce_graphics.mm
        ${JUCE_LIB}/include_juce_gui_basics.mm
        ${JUCE_LIB}/include_juce_gui_extra.mm
        ${JUCE_LIB}/include_juce_audio_processors.mm
        ${JUCE_LIB}/include_juce_audio_plugin_client_VST_utils.mm
    )
else()
    set(JUCE_SRC
        ${JUCE_SRC}
        ${JUCE_LIB}/include_juce_data_structures.cpp
        ${JUCE_LIB}/include_juce_events.cpp
        ${JUCE_LIB}/include_juce_audio_basics.cpp
        ${JUCE_LIB}/include_juce_core.cpp
        ${JUCE_LIB}/include_juce_opengl.cpp
        ${JUCE_LIB}/include_juce_graphics.cpp
        ${JUCE_LIB}/include_juce_gui_basics.cpp
        ${JUCE_LIB}/include_juce_gui_extra.cpp
        ${JUCE_LIB}/include_juce_audio_processors.cpp
    )
endif()

if(APPLE)
    FIND_LIBRARY(ACCELERATE_LIB Accelerate)
    FIND_LIBRARY(COCOA_LIB Cocoa)
    FIND_LIBRARY(COREIMAGE_LIB CoreImage)
    FIND_LIBRARY(IOKIT_LIB IOkit)
    FIND_LIBRARY(WEBKIT_LIB WebKit)
    include(FindOpenGL)
    set(JUCE_DEPS
        ${ACCELERATE_LIB}
        ${COCOA_LIB}
        ${COREIMAGE_LIB}
        ${IOKIT_LIB}
        ${WEBKIT_LIB}
        ${OPENGL_LIBRARIES}
        PARENT_SCOPE
    )
elseif(UNIX)
    find_package(PkgConfig REQUIRED)

    function(add_package prefix pkg)
        pkg_search_module(${prefix} REQUIRED ${pkg})
        include_directories(${${prefix}_INCLUDE_DIRS})
        link_directories(${${prefix}_LIBRARY_DIRS})
        add_definitions(${${prefix}_CFLAGS_OTHER})
    endfunction()

    add_package(ALSA alsa)
    add_package(FREETYPE2 freetype2)
    add_package(GTK+X11 gtk+-x11-3.0)
    add_package(CURL libcurl)
    add_package(WEBKIT2GTK webkit2gtk-4.0)
    add_package(X11 x11)
    add_package(XEXT xext)
    add_package(XINERAMA xinerama)

else()
endif()

add_library(juce_lib_obj OBJECT ${JUCE_SRC})
set_target_properties(juce_lib_obj PROPERTIES COMPILE_FLAGS "-fPIC")
