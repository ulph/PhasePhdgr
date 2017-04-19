#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrScope.h"
#include "DirectoryWatcher.hpp"
#include <vector>

class PhasePhckrAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    PhasePhckrAudioProcessorEditor (PhasePhckrAudioProcessor&);
    ~PhasePhckrAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    PhasePhckrAudioProcessor& processor;
    PhasePhckrScope voiceScopeL;
    PhasePhckrScope outputScopeL;
    PhasePhckrScope voiceScopeR;
    PhasePhckrScope outputScopeR;
    TabbedComponent mainFrame;
    PhasePhckrGrid scopeGrid;
    PhasePhckrGrid editorGrid;

    TextEditor voiceEditor;
    TextEditor effectEditor;
    PhasePhckrGrid editorMenu;

    DirectoryContentsList voiceDirectoryWatcher;
    DirectoryContentsList effectDirectoryWatcher;

    FileListComponent voiceDirectoryList;
    FileListComponent effectDirectoryList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
