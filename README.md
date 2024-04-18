# README #

PhasePhdgr is an (extendable) MPE capabable software module synthesizer. It comes both as a headless CLI (jack) synth, and wrapped inside an elaborate VST/GUI (JUCE) in two flavours (instrument and effect).

Under the hood, all connections and states are described via hiarchies in JSON. A user can create subgraphs, optionally storing it to disk, and reference it from other (sub)graphs. Advanced users can extend the synth by using the minimal sdk provided (as opposed to contributing to this repo).

## Highlights ##
- Cross platform plugin (VST3, AU) and standalone. Courtesy of using JUCE.
- Sample-by-sample feedback processing in the modular compute graph
- MPE support
- Graph descriptions stored on disk as JSON, ie editable by external means!
- Building blocks like ZDFs (zero delay filters) and an oscillator that handles (partial sync). Delay lines with support for fractional (subsample precision) and modulated delay.
- Experiment oriented versions of graph components where the state is an output/input set. Allows for quick experimentation with nonlinearities in filter (internal) feedback paths, for instance.

## Disiclaimers ##
This is an old hobby project of mine (and some friends). It does NOT represent a sensible software project. The UI bits are partly broken and horribly implemented. There are few unit tests. There are however some DSP nuggets, and the graphing engine is quite clever and useful. There are also plans to polish the quality of testing and  a complete rewrite of the UI (not using JUCE).

## build targets ##

* core / synth / tools - buildable from from ./core
* VST / JUCE - option BUILD_JUCE
* CLI client(s) - option BUILD_CLI
* Plugin SDK export - option BUILD_PLUGIN_SDK

## submodules ##

* nlhomann/json - forked version to tweak the pretty print behaviour
* ThreadPool - forked to add a namespace
* VST3 SDK - note, is a collection of submodules itself
* JUCE
