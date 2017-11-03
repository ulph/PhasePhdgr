#include <cstring>
#include <typeinfo>

#include <phasephckr.hpp>
#include <phasephckr_json.hpp>
#include "JuceHeader.h"

#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrPluginEditor.h"
#include "DirectoryWatcher.hpp"

using namespace PhasePhckrFileStuff;

using namespace std;

static json loadJson(const File & f) {
    String s = f.loadFileAsString();
    return json::parse(s.toStdString().c_str());
}

PhasePhckrAudioProcessorEditor::PhasePhckrAudioProcessorEditor(PhasePhckrAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
    , voiceScopeL(processor.getSynth()->getVoiceScope(0))
    , voiceScopeR(processor.getSynth()->getVoiceScope(1))
    , voiceScopeXY(processor.getSynth()->getVoiceScope(0), processor.getSynth()->getVoiceScope(1))
    , outputScopeL(processor.getSynth()->getOutputScope(0))
    , outputScopeR(processor.getSynth()->getOutputScope(1))
    , outputScopeXY(processor.getSynth()->getOutputScope(0), processor.getSynth()->getOutputScope(1))
    , mainFrame(TabbedButtonBar::TabsAtTop)

    , voiceEditor(
        processor.activeVoice,
        processor.subComponentRegister,
        c_voiceChainInBus,
        c_voiceChainOutBus
    )

    , effectEditor(
        processor.activeEffect,
        processor.subComponentRegister,
        c_effectChainInBus,
        c_effectChainOutBus
    )

    , fileWatchThread("editorFileWatchThread")
    , voiceFiles(
        PhasePhckrFileStuff::voicesDir,
        fileWatchThread,
        [this](const File& f) {
            processor.activeVoice.set(-1, loadJson(f));
        }
    )
    , effectFiles(
        PhasePhckrFileStuff::effectsDir,
        fileWatchThread, 
        [this](const File& f) {
            processor.activeEffect.set(-1, loadJson(f));
        }
    )
    , patchFiles(
        PhasePhckrFileStuff::patchesDir,
        fileWatchThread, 
        [this](const File& f){} // TODO, routine to load a patch, simply ...
    )
    , componentFiles(
        PhasePhckrFileStuff::componentsDir,
        fileWatchThread, 
        [this](const File& f) {}
    )

#if INTERCEPT_STD_STREAMS
    , coutIntercept(std::cout)
    , cerrIntercept(std::cerr)
#endif
    , guiUpdateTimer(new function<void()>([this](){
        for(const auto &knob : parameterKnobs){
            knob->update();
        }
        voiceScopeL.repaint();
        voiceScopeR.repaint();
        voiceScopeXY.repaint();
        outputScopeL.repaint();
        outputScopeR.repaint();
        outputScopeXY.repaint();
    }))
{

    fileWatchThread.startThread();
    fileWatchThread.notify();

    setResizeLimits(128, 128, 1800, 1000);
    setConstrainer(nullptr);
    setResizable(true, true);
    setBoundsConstrained(Rectangle<int>(1800, 1000)); // slightly less than 1080p
    addAndMakeVisible(mainFrame);

    mainFrame.addTab("scopes", Colours::black, &scopeGrid, false);
    scopeGrid.addComponent(&voiceScopeL);
    scopeGrid.addComponent(&voiceScopeXY);
    scopeGrid.addComponent(&voiceScopeR);
    scopeGrid.addComponent(&outputScopeL);
    scopeGrid.addComponent(&outputScopeXY);
    scopeGrid.addComponent(&outputScopeR);
    scopeGrid.setColoumns({0.33f, 0.33f, 0.33f});

    mainFrame.addTab("patch", Colours::black, &performGrid, false);
    performGrid.setColoumns({ 1.f ,1.f ,1.f ,1.f, 1.f, 1.f ,1.f ,1.f });
    for(int i=0; i<processor.numberOfParameters(); i++){
        PhasePhckrParameter* p = nullptr;
        if(processor.accessParameter(i, &p)){
            auto knob = new ParameterKnob(p,
                [this](string a, string b){
                    processor.swapParameterIndices(a, b);
                    guiUpdateTimer.timerCallback();
                }
            );
            parameterKnobs.push_back(knob);
            performGrid.addComponent(knob);
        }
    }

    mainFrame.addTab("voice", Colours::black, &voiceEditor, false);
    mainFrame.addTab("effect", Colours::black, &effectEditor, false);

    mainFrame.addTab("files", Colours::black, &filesGrid, false);
    filesGrid.addComponent(&voiceFiles.list);
    filesGrid.addComponent(&effectFiles.list);
    filesGrid.addComponent(&patchFiles.list);
    filesGrid.addComponent(&componentFiles.list);

#if INTERCEPT_STD_STREAMS
    mainFrame.addTab("debug", Colours::black, &debugTab, false);
    debugTab.addComponent(&coutView);
    debugTab.addComponent(&cerrView);
    coutView.setMultiLine(true, true);
    cerrView.setMultiLine(true, true);
    debugViewUpdateTimer = new LambdaTimer(new std::function<void()>([this](){
        std::string s;
        coutIntercept.readAll(s);
        if (s.size()) {
            coutView.insertTextAtCaret(s);
        }
        s.clear();
        cerrIntercept.readAll(s);
        if (s.size()) {
            cerrView.insertTextAtCaret(s);
        }
    }));
    debugViewUpdateTimer->startTimer(1000);
#endif

    processor.broadcastPatch();

    guiUpdateTimer.startTimer((int)(1.f/60.f * 1000.f));

    resized();
}

PhasePhckrAudioProcessorEditor::~PhasePhckrAudioProcessorEditor()
{
    guiUpdateTimer.stopTimer();
    for(const auto &knob : parameterKnobs){
        delete knob;
    }
#if INTERCEPT_STD_STREAMS
    debugViewUpdateTimer->stopTimer();
    delete debugViewUpdateTimer;
#endif
}

void PhasePhckrAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colours::black);
}

void PhasePhckrAudioProcessorEditor::resized()
{
    mainFrame.setBoundsRelative(0, 0, 1, 1);
    repaint();
}
