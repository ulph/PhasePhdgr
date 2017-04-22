# README #

Read my lips...
Suck my balls!

## core / synth ##

As long as you work with the JUCE-less parts, leave cmake options at default.

## json format ##

We rely on nlhomann/json for this. 

* Copy that projects json.hpp to SOMEPATH/nlohmann (or clone their repo). 
* cmake options USING_NLOHMANN_JSON=ON and JSON_INCLUDE_DIR=SOMEPATH

## JUCE ##

A few extra steps (done first time only). After that it's just CMAKE!

Use Projucer to generate some dummy project files (which we don't care about) as well as some wrapper cpp/header files.

* see section *json format*

* Download JUCE sdk or clone from their repo. We need Projucer binary at least once, so maybe easiest to grab the SDK download prebuilt.

* Symlink the sdk root into our source tree as JUCE/JUCE. This is because Projucer needs absolute or relative paths, and cannot use any enviromental variables...

* Run Projucer, open the PhasePhckr.juser file. 

* Save inside Projucer (ctrl s). Folder JuceLibraryCode appears.

* Run cmake with BUILD_JUCE_STUFF=ON.

We should be good now, and from here on you don't need to care with Projucer etc.