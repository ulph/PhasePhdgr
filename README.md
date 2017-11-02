# README #

Read my lips... Suck my balls!


## core / synth / tools ##

See "json" below.

Run cmake from #core.


## VST / JUCE ##

Run cmake from the root.

Download or clone JUCE SDK and VST3 SDK. There's an option for either in our CMAKE, fill those in...

If updating the JUCE bits open JUCE/PhasePhckr.jucer with projucer and save. Ignore the generated project, but do commit any changes to the generated stubs.


## json ##

We rely on nlhomann/json for this. 

Our code expects nlhomann/json.hpp in include path. There is a field in our CMAKE for it.
