# README #

Read my lips... Suck my balls!


## core / synth / tools ##

See "json" below.

Run cmake from ./core


## VST / JUCE ##

Run cmake from the root.

Download or clone JUCE SDK and VST3 SDK. There's an option for location either in our CMAKE.

If updating the JUCE bits open JUCE/PhasePhckr.jucer with projucer and save. Ignore the generated project, but do commit any changes to the generated stubs.


## submodules ##

### json ###
We rely on nlhomann/json for this. We use a forked version to tweak the pretty print behaviour.

### threadpool ###

We rely on a threadpool implementation of this. Note, forked to add a namespace.
