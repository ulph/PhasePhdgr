function(make_juce_effect_lib)
    add_definitions(-DJucePlugin_Build_VST=1)
    add_definitions(-DJucePlugin_Build_VST3=1)
    add_definitions(-DJucePlugin_WantsMidiInput=0)
    add_definitions(-DJucePlugin_IsSynth=0)
    add_definitions(-DJucePlugin_Name="PhasePhckrFX") # override
    add_definitions(-JucePlugin_PluginCode=0x4f6f6162) # 0x4f6f6161 + 1

    make_juce_lib(juce_effect_lib_obj)
endfunction(make_juce_effect_lib)
make_juce_effect_lib()
