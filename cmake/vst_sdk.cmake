# Copy paste (and modify) from VST SDK, as JUCE did not play nice ...

set(VST3_OUTPUT_DIR ${CMAKE_BINARY_DIR}/VST3 CACHE FILEPATH "Where to put the VST3")

function(mod_smtg_set_exported_symbols target exported_symbols_file)
    if(MSVC)
        set_target_properties(${target} PROPERTIES LINK_FLAGS "/DEF:\"${exported_symbols_file}\"")
    elseif(XCODE)
        set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE "${exported_symbols_file}")
    else()
        set_target_properties(${target} PROPERTIES LINK_FLAGS "-exported_symbols_list \"${exported_symbols_file}\"")
    endif()
endfunction()

function(mod_smtg_add_vst3plugin target sdkroot)
    set(sources ${ARGN})
    add_library(${target} MODULE ${sources})
    set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${VST3_OUTPUT_DIR})
    if(APPLE)
        set_target_properties(${target} PROPERTIES BUNDLE TRUE)
        if(XCODE)
            set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES")
            set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_WRAPPER_EXTENSION "vst3")
        else()
            set_target_properties(${target} PROPERTIES BUNDLE_EXTENSION "vst3")
        endif()
        mod_smtg_set_exported_symbols(${target} "${sdkroot}/public.sdk/source/main/macexport.exp")
    elseif(WIN)
        set_target_properties(${target} PROPERTIES SUFFIX ".vst3")
        set_target_properties(${target} PROPERTIES LINK_FLAGS "/EXPORT:GetPluginFactory")
    elseif(LINUX)
        EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
        set(target_lib_dir ${ARCHITECTURE}-linux)
        set_target_properties(${target} PROPERTIES PREFIX "")
        set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${VST3_OUTPUT_DIR}/${target}.vst3/Contents/${target_lib_dir}")
        add_custom_command(TARGET ${target} PRE_LINK
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "${VST3_OUTPUT_DIR}/${target}.vst3/Contents/Resources"
        )
    endif()
endfunction()

function(mod_smtg_add_vst3_resource target input_file)
    if (LINUX)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_LIST_DIR}/${input_file}"
            "${VST3_OUTPUT_DIR}/${target}.vst3/Contents/Resources/"
        )
    elseif(APPLE)
        target_sources(${target} PRIVATE ${input_file})
        set_source_files_properties(${input_file} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    endif(LINUX)
endfunction()
