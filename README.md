# README #

Read my lips...
Suck my balls!

Run cmake from either
* root, if you want all the stuff

* core, if you want just the core (no gui/vst)


## core / synth ##

As long as you work with the JUCE-less parts, leave cmake options at default.

## json format ##

We rely on nlhomann/json for this. 

Our code expects nlhomann/json.hpp in include path. There is a field in our CMAKE for it.

## VST / JUCE ##

Download or clone JUCE SDK and VST3 SDK. There's an option for either in our CMAKE, fill those in...

### Notes ###

* We have the autogenerated define+source includes from Projucer checked in. We _could_ potentially avoid that, but the defines in Appconfig.h would then have to be set by CMAKE, and the files are 2-liners anyway typically...