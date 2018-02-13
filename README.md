# README #

PhasePhckr is a (extendable) software module synthesizer.

# Use #

...

## Patch structure ##

### Presets ###

### Patch ###

### Component ###

Global vs local.

## Variants ##

### JUCE/VST ###
Synth version, FX version.

### CLI ###
Jack.

# Extend #

## JSON/Graphs ##

Just use the software, and build a library of COMPONENTS, VOICE/EFFECT PATCHES and PRESETS in nested folder structures.

## Binary/Nodes ##

Use the minimal plugin SDK to create a dynamic library that defines a bunch of new processing nodes (Modules).

# Develop #

Core synth should build on c++11 compliant platforms, including OSX, Linux, and Windows for both x86 and ARM.

JUCE/VST layer should build for OSX, Linux and Windows X86.

## build targets ##

### core / synth / tools ###

See "json" below.

Run cmake from ./core

### VST / JUCE ###

Run cmake from the root, enable option BUILD_JUCE

### CLI client(s) ###

Run cmake from the root, enable option BUILD_CLI

### Plugin SDK export ###

Run cmake from the root, enable option BUILD_PLUGIN_SDK

## submodules ##

* nlhomann/json - forked version to tweak the pretty print behaviour
* ThreadPool - forked to add a namespace
* VST3 SDK
* JUCE
