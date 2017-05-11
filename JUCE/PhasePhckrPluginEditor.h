#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "JuceLibraryCode/JuceHeader.h"
#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrScope.h"
#include "DirectoryWatcher.hpp"
#include "GraphView.h"
#include "docs.hpp"
#include <vector>
#include "Utils.hpp"
#include "GraphEditor.hpp"


class PhasePhckrAudioProcessorEditor  : public AudioProcessorEditor, public DragAndDropContainer
{
public:
    PhasePhckrAudioProcessorEditor (
        PhasePhckrAudioProcessor&,
        SubValue<PatchDescriptor> &activeVoice,
        SubValue<PatchDescriptor> &activeEffect
     );
    ~PhasePhckrAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    PhasePhckrAudioProcessor& processor;

    SubValue<PatchDescriptor> &activeVoice;
    SubValue<PatchDescriptor> &activeEffect;
    int activeVoiceSubscribeHandle;
    int activeEffectSubscribeHandle;

    PhasePhckrScope voiceScopeL;
    PhasePhckrScope voiceScopeR;
    PhasePhckrXYScope voiceScopeXY;
    PhasePhckrScope outputScopeL;
    PhasePhckrScope outputScopeR;
    PhasePhckrXYScope outputScopeXY;

    TabbedComponent mainFrame;
    PhasePhckrGrid scopeGrid;
    PhasePhckrGrid performGrid;

    TimeSliceThread fileWatchThread;
    DirectoryContentsList voiceDirectoryWatcher;
    FileListComponent voiceDirectoryList;
    PhasePhckrFileStuff::StupidFileBrowserListener voiceListListener;
    DirectoryContentsList effectDirectoryWatcher;
    FileListComponent effectDirectoryList;
    PhasePhckrFileStuff::StupidFileBrowserListener effectListListener;
    PhasePhckrGrid filesGrid;
    LambdaTimer fileUpdateTimer;
    void updateFiles() {
        voiceDirectoryWatcher.refresh();
        effectDirectoryWatcher.refresh();
    }

    GraphEditor voiceEditor;
    GraphEditor effectEditor;

#if INTERCEPT_STD_STREAMS
    InterceptStringStream coutIntercept;
    InterceptStringStream cerrIntercept;
    TextEditor coutView;
    TextEditor cerrView;
    PhasePhckrGrid debugTab;
    LambdaTimer* debugViewUpdateTimer;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
