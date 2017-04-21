#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrScope.h"
#include "DirectoryWatcher.hpp"
#include "GraphView.h"
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
    PhasePhckrScope voiceScopeR;
    PhasePhckrXYScope voiceScopeXY;
    PhasePhckrScope outputScopeL;
    PhasePhckrScope outputScopeR;
    PhasePhckrXYScope outputScopeXY;

    TabbedComponent mainFrame;
    PhasePhckrGrid scopeGrid;
    PhasePhckrGrid editorGrid;

    TextEditor voiceEditor;
    TextEditor effectEditor;
    PhasePhckrGrid editorMenu;

    PhasePhckrGrid performGrid;

    DirectoryContentsList voiceDirectoryWatcher;
    FileListComponent voiceDirectoryList;
    PhasePhckrFileStuff::StupidFileBrowserListener voiceListListener;

    DirectoryContentsList effectDirectoryWatcher;
    FileListComponent effectDirectoryList;
    PhasePhckrFileStuff::StupidFileBrowserListener effectListListener;

    DirectoryContentsList componentDirectoryWatcher;
    FileListComponent componentDirectoryList;
    PhasePhckrFileStuff::StupidFileBrowserListener componentListListener;

    DirectoryContentsList patchDirectoryWatcher;
    FileListComponent patchDirectoryList;
    PhasePhckrFileStuff::StupidFileBrowserListener patchListListener;

    Viewport voiceGraphViewport;
    GraphView voiceGraphView;
    
    Viewport effectGraphViewport;
    GraphView effectGraphView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
