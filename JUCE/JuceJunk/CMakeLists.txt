include_directories(${JUCE_SDK_ROOT}/modules)

set(JUCE_JUNK ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode)
include_directories(${JUCE_JUNK})

function(make_juce_lib lib_target)

    set(JUCE_SRC
        ${JUCE_JUNK}/AppConfig.h
        ${JUCE_JUNK}/JuceHeader.h
        ${JUCE_JUNK}/include_juce_audio_plugin_client_utils.cpp
        ${JUCE_JUNK}/include_juce_audio_plugin_client_VST2.cpp
    #    ${JUCE_JUNK}/include_juce_audio_plugin_client_VST3.cpp
    )

    if(APPLE)
        add_definitions(-x objective-c++)
        set(JUCE_SRC
            ${JUCE_SRC}
            ${JUCE_JUNK}/include_juce_data_structures.mm
            ${JUCE_JUNK}/include_juce_events.mm
            ${JUCE_JUNK}/include_juce_audio_basics.mm
            ${JUCE_JUNK}/include_juce_core.mm
            ${JUCE_JUNK}/include_juce_opengl.mm
            ${JUCE_JUNK}/include_juce_graphics.mm
            ${JUCE_JUNK}/include_juce_gui_basics.mm
            ${JUCE_JUNK}/include_juce_gui_extra.mm
            ${JUCE_JUNK}/include_juce_audio_processors.mm
            ${JUCE_JUNK}/include_juce_audio_plugin_client_VST_utils.mm
        )
    else()
        set(JUCE_SRC
            ${JUCE_SRC}
            ${JUCE_JUNK}/include_juce_data_structures.cpp
            ${JUCE_JUNK}/include_juce_events.cpp
            ${JUCE_JUNK}/include_juce_audio_basics.cpp
            ${JUCE_JUNK}/include_juce_core.cpp
            ${JUCE_JUNK}/include_juce_opengl.cpp
            ${JUCE_JUNK}/include_juce_graphics.cpp
            ${JUCE_JUNK}/include_juce_gui_basics.cpp
            ${JUCE_JUNK}/include_juce_gui_extra.cpp
            ${JUCE_JUNK}/include_juce_audio_processors.cpp
        )
    endif()

    add_definitions(-DJUCE_SHARED_CODE=1)

    if(APPLE)

        add_definitions(-DMAC=1)
        add_definitions(-DJUCER_XCODE_MAC_F6D2F4CF=1)

        FIND_LIBRARY(ACCELERATE_LIB Accelerate)
        FIND_LIBRARY(COCOA_LIB Cocoa)
        FIND_LIBRARY(COREIMAGE_LIB CoreImage)
        FIND_LIBRARY(IOKIT_LIB IOkit)
        FIND_LIBRARY(WEBKIT_LIB WebKit)
        FIND_LIBRARY(COREFOUNDATION_LIB CoreFoundation)
        FIND_LIBRARY(CORESERVICES_LIB CoreServices)
        FIND_LIBRARY(OPENGL_LIB OpenGL)
#        include(FindOpenGL)
        set(JUCE_DEPS
            ${ACCELERATE_LIB}
            ${COCOA_LIB}
            ${COREIMAGE_LIB}
            ${IOKIT_LIB}
            ${WEBKIT_LIB}
            ${COREFOUNDATION_LIB}
            ${CORESERVICES_LIB}
            ${OPENGL_LIB}
#            ${OPENGL_LIBRARIES}
            CACHE
            INTERNAL
            ""
        )
    elseif(UNIX)

        set(common_linker_flags "-Wl,--no-undefined")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${common_linker_flags}")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${common_linker_flags}")

        add_definitions(-DLINUX=1)
        add_definitions(-DJUCER_LINUX_MAKE_7346DA2A=1)

        include(FindOpenGL)
        find_package(Threads)

        set(JUCE_DEPS
            ${CMAKE_THREAD_LIBS_INIT}
            ${OPENGL_LIBRARIES}
            ${CMAKE_DL_LIBS}
        )

        find_package(PkgConfig REQUIRED)

        function(add_package prefix pkg list)
            pkg_search_module(${prefix} REQUIRED ${pkg})
            include_directories(${${prefix}_INCLUDE_DIRS})
            link_directories(${${prefix}_LIBRARY_DIRS})
            add_definitions(${${prefix}_CFLAGS_OTHER})
            set(${list}
                ${${list}}
                ${${prefix}_LIBRARIES}
                PARENT_SCOPE
            )
        endfunction()

        add_package(ALSA alsa JUCE_DEPS)
        add_package(FREETYPE2 freetype2 JUCE_DEPS)
        add_package(GTK+X11 gtk+-x11-3.0 JUCE_DEPS)
        add_package(CURL libcurl JUCE_DEPS)
        add_package(WEBKIT2GTK webkit2gtk-4.0 JUCE_DEPS)
        add_package(X11 x11 JUCE_DEPS)
        add_package(XEXT xext JUCE_DEPS)
        add_package(XINERAMA xinerama JUCE_DEPS)

        set(JUCE_DEPS
            ${JUCE_DEPS}
            PARENT_SCOPE
            CACHE
            INTERNAL
            ""
        )

    else()
    endif()

    add_library(${lib_target} OBJECT ${JUCE_SRC})
    set_target_properties(${lib_target} PROPERTIES COMPILE_FLAGS "-fPIC")
    set_target_properties(${lib_target} PROPERTIES FOLDER JUCE/JUCE)

endfunction(make_juce_lib)

add_subdirectory(Synth)
add_subdirectory(Effect)
