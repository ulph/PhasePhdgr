# README #

PhasePhckr is a (extendable) software module synthesizer. It comes both as a headless CLI (jack) synth, and wrapped inside an elaborate VST/GUI (JUCE) in two flavours (instrument and effect).

Under the hood, all connections and states are described via hiarchies in JSON. A user can create subgraphs, optionally storing it to disk, and reference it from other (sub)graphs. Advanced users can extend the synth by using the minimal sdk provided.

## build targets ##

* core / synth / tools - buildable from from ./core
* VST / JUCE - option BUILD_JUCE
* CLI client(s) - option BUILD_CLI
* Plugin SDK export - option BUILD_PLUGIN_SDK

## submodules ##

* nlhomann/json - forked version to tweak the pretty print behaviour
* ThreadPool - forked to add a namespace
* VST3 SDK
* JUCE
