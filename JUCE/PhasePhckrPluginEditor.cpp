#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrPluginEditor.h"
#include "phasephckr.h"
#include <cstring>
#include "DirectoryWatcher.hpp"
#include "Style.hpp"
#include <typeinfo>

PhasePhckrLookAndFeel g_lookAndFeel;

using namespace PhasePhckrFileStuff;

const Font monoFont = Font(Font::getDefaultMonospacedFontName(), 14, 0);

static json loadJson(const File & f) {
    String s = f.loadFileAsString();
    return json::parse(s.toStdString().c_str());
}

PhasePhckrAudioProcessorEditor::PhasePhckrAudioProcessorEditor (PhasePhckrAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
    , voiceScopeL(processor.getSynth()->getVoiceScope(0))
    , voiceScopeR(processor.getSynth()->getVoiceScope(1))
    , outputScopeL(processor.getSynth()->getOutputScope(0))
    , outputScopeR(processor.getSynth()->getOutputScope(1))
    , voiceScopeXY(processor.getSynth()->getVoiceScope(0), processor.getSynth()->getVoiceScope(1))
    , outputScopeXY(processor.getSynth()->getOutputScope(0), processor.getSynth()->getOutputScope(1))
    , mainFrame(TabbedButtonBar::TabsAtTop)
    , voiceGraphViewport("voiceGraphView")
    , effectGraphViewport("effectGraphView")

    , voiceDirectoryWatcher(PhasePhckrFileStuff::getFilter(), PhasePhckrFileStuff::getThread())
    , effectDirectoryWatcher(PhasePhckrFileStuff::getFilter(), PhasePhckrFileStuff::getThread())
    
    , voiceDirectoryList(voiceDirectoryWatcher)
    , effectDirectoryList(effectDirectoryWatcher)

    , activeVoiceSubscribeHandle(activeVoice.subscribe(
        [this](const PhasePhckr::ConnectionGraphDescriptor & v) {
            processor.setVoicePatch(v);
        }))
    , activeEffectSubscribeHandle(activeEffect.subscribe(
        [this](const PhasePhckr::ConnectionGraphDescriptor & v) {
            processor.setEffectPatch(v);
        }))
    , voiceListListener([this](const File& f) { 
            activeVoice.set(-1, loadJson(f));
        })
    , effectListListener([this](const File& f) { 
            activeEffect.set(-1, loadJson(f));
        })

    , doc(processor.componentRegister)
    , docListModel(doc.get(), docView)
    , docList( "docList", &docListModel)

    , voiceEditor(&activeVoice)
    , effectEditor(&activeEffect)

    , voiceGraphView(doc, &activeVoice)
    , effectGraphView(doc, &activeEffect)

    , coutIntercept(std::cout)
    , cerrIntercept(std::cerr)
{

    activeVoice.set(-1, processor.getVoicePatch());
    activeEffect.set(-1, processor.getEffectPatch());

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
    scopeGrid.setNumberOfColumns(3);

    mainFrame.addTab("perform", g_tabColor, &performGrid, false);

    mainFrame.addTab("voice graph", g_tabColor, &voiceGraphViewport, false);
    voiceGraphViewport.setViewedComponent(&voiceGraphView, false);

    mainFrame.addTab("effect graph", g_tabColor, &effectGraphViewport, false);
    effectGraphViewport.setViewedComponent(&effectGraphView, false);

    mainFrame.addTab("files", g_tabColor, &editorGrid, false);
    editorGrid.addComponent(&voiceEditor);
    editorGrid.addComponent(&editorMenu);
    editorGrid.addComponent(&effectEditor);
    editorGrid.setNumberOfColumns(3);
    editorMenu.addComponent(&voiceDirectoryList);
    editorMenu.addComponent(&effectDirectoryList);
    editorMenu.addComponent(&docList);
    editorMenu.addComponent(&docView);
    editorMenu.setNumberOfColumns(2);
    docList.updateContent();
    docView.setMultiLine(true, true);

    voiceDirectoryList.addListener(&voiceListListener);
    effectDirectoryList.addListener(&effectListListener);

    voiceDirectoryWatcher.setDirectory(PhasePhckrFileStuff::voicesDir, true, true);
    effectDirectoryWatcher.setDirectory(PhasePhckrFileStuff::effectsDir, true, true);

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
    std::cout << "#intercepted cout#\n\n" << std::endl;
    std::cerr << "#intercepted cerr#\n\n" << std::endl;

    resized();
}

PhasePhckrAudioProcessorEditor::~PhasePhckrAudioProcessorEditor()
{
    delete debugViewUpdateTimer;
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
