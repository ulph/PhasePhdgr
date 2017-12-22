add_definitions(-DJucePlugin_Build_VST=1)
add_definitions(-DJucePlugin_Build_VST3=1)
add_definitions(-DJucePlugin_WantsMidiInput=1)
add_definitions(-DJucePlugin_IsSynth=1)

make_juce_lib(juce_synth_obj)
