add_definitions(-DJucePlugin_Build_VST=1)
add_definitions(-DJucePlugin_Build_VST3=0)
add_definitions(-DJucePlugin_WantsMidiInput=0)
add_definitions(-DJucePlugin_IsSynth=0)
add_definitions(-DJucePlugin_Name="PhasePhckrFX") # override
add_definitions(-DJucePlugin_Desc="PhasePhckrFX") # override
add_definitions(-DJucePlugin_VSTCategory=kPlugCategEffect) # override
add_definitions(-DJucePlugin_PluginCode=0x4f6f6162) # 0x4f6f6161 + 1

make_juce_lib(juce_effect_obj)
