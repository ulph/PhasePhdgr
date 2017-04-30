#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrPluginEditor.h"
#include "phasephckr.h"
#include <cstring>
#include "DirectoryWatcher.hpp"
#include "Style.hpp"
#include <typeinfo>
#include "docs.hpp"

PhasePhckrLookAndFeel g_lookAndFeel;

using namespace PhasePhckrFileStuff;

static json loadJson(const File & f) {
    String s = f.loadFileAsString();
    return json::parse(s.toStdString().c_str());
}

PhasePhckrAudioProcessorEditor::PhasePhckrAudioProcessorEditor (
        PhasePhckrAudioProcessor& p,
        SubValue<PhasePhckr::ConnectionGraphDescriptor> &activeVoice_,
        SubValue<PhasePhckr::ConnectionGraphDescriptor> &activeEffect_
        )
    : AudioProcessorEditor (&p), processor (p)
    , activeVoice(activeVoice_)
    , activeEffect(activeEffect_)
    , voiceScopeL(processor.getSynth()->getVoiceScope(0))
    , voiceScopeR(processor.getSynth()->getVoiceScope(1))
    , outputScopeL(processor.getSynth()->getOutputScope(0))
    , outputScopeR(processor.getSynth()->getOutputScope(1))
    , voiceScopeXY(processor.getSynth()->getVoiceScope(0), processor.getSynth()->getVoiceScope(1))
    , outputScopeXY(processor.getSynth()->getOutputScope(0), processor.getSynth()->getOutputScope(1))
    , mainFrame(TabbedButtonBar::TabsAtTop)

    , voiceDirectoryWatcher(PhasePhckrFileStuff::getFilter(), PhasePhckrFileStuff::getThread())
    , effectDirectoryWatcher(PhasePhckrFileStuff::getFilter(), PhasePhckrFileStuff::getThread())
    
    , voiceDirectoryList(voiceDirectoryWatcher)
    , effectDirectoryList(effectDirectoryWatcher)

    , activeVoiceSubscribeHandle(activeVoice.subscribe(
        [this](const PhasePhckr::ConnectionGraphDescriptor & v) {
            //
        }))
    , activeEffectSubscribeHandle(activeEffect.subscribe(
        [this](const PhasePhckr::ConnectionGraphDescriptor & v) {
            //
        }))
    , voiceListListener([this](const File& f) {
            activeVoice.set(activeVoiceSubscribeHandle, loadJson(f));
        })
    , effectListListener([this](const File& f) {
            activeEffect.set(activeEffectSubscribeHandle, loadJson(f));
        })

    , voiceEditor(
            [this]()->PhasePhckr::Doc 
            {
                PhasePhckr::Doc d(processor.componentRegister);
                d.add(PhasePhckr::getVoiceBusInputDoc());
                d.add(PhasePhckr::getVoiceBusOutputDoc());
                return d;
            }(),
            activeVoice,
            c_VoiceInput,
            c_VoiceOutput
        )
    , effectEditor(
            [this]()->PhasePhckr::Doc 
            {
                PhasePhckr::Doc d(processor.componentRegister);
                d.add(PhasePhckr::getEffectBusInputDoc());
                d.add(PhasePhckr::getEffectBusOutputDoc());
                return d;
            }(),
            activeEffect,
            c_EffectInput,
            c_EffectOutput
        )
#if INTERCEPT_STD_STREAMS
    , coutIntercept(std::cout)
    , cerrIntercept(std::cerr)
#endif
{

    setLookAndFeel(&g_lookAndFeel);
    setResizeLimits(128, 128, 1800, 1000);
    setConstrainer(nullptr);
    setResizable(true, true);
    setBoundsConstrained(Rectangle<int>(1800, 1000)); // slightly less than 1080p
    addAndMakeVisible(mainFrame);

    mainFrame.addTab("scopes", g_tabColor, &scopeGrid, false);
    scopeGrid.addComponent(&voiceScopeL);
    scopeGrid.addComponent(&voiceScopeXY);
    scopeGrid.addComponent(&voiceScopeR);
    scopeGrid.addComponent(&outputScopeL);
    scopeGrid.addComponent(&outputScopeXY);
    scopeGrid.addComponent(&outputScopeR);
    scopeGrid.setColoumns({0.33f, 0.33f, 0.33f});

    mainFrame.addTab("parameters", g_tabColor, &performGrid, false);
    mainFrame.addTab("voice", g_tabColor, &voiceEditor, false);
    mainFrame.addTab("effect", g_tabColor, &effectEditor, false);

    mainFrame.addTab("files", g_tabColor, &filesGrid, false);
    filesGrid.addComponent(&voiceDirectoryList);
    filesGrid.addComponent(&effectDirectoryList);
    voiceDirectoryList.addListener(&voiceListListener);
    effectDirectoryList.addListener(&effectListListener);
    voiceDirectoryWatcher.setDirectory(PhasePhckrFileStuff::voicesDir, true, true);
    effectDirectoryWatcher.setDirectory(PhasePhckrFileStuff::effectsDir, true, true);

#if INTERCEPT_STD_STREAMS
    mainFrame.addTab("debug", g_tabColor, &debugTab, false);
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

    resized();
}

PhasePhckrAudioProcessorEditor::~PhasePhckrAudioProcessorEditor()
{
    activeEffect.unsubscribe(activeEffectSubscribeHandle);
    activeVoice.unsubscribe(activeVoiceSubscribeHandle);
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
