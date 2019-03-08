#include <cstring>
#include <typeinfo>

#include <phasephckr.hpp>
#include <phasephckr_json.hpp>
#include "JuceHeader.h"

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DirectoryWatcher.hpp"

#include "FileIO.hpp"

using namespace PhasePhckrFileStuff;

using namespace std;

PhasePhckrEditor::PhasePhckrEditor(PhasePhckrProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
    , voiceScopeL(processor.getProcessor(SynthGraphType::VOICE)->getVoiceScope(0))
    , voiceScopeR(processor.getProcessor(SynthGraphType::VOICE)->getVoiceScope(1))
    , voiceScopeXY(processor.getProcessor(SynthGraphType::VOICE)->getVoiceScope(0), processor.getProcessor(SynthGraphType::VOICE)->getVoiceScope(1))
    , synthScopeL(processor.getProcessor(SynthGraphType::VOICE)->getOutputScope(0))
    , synthScopeR(processor.getProcessor(SynthGraphType::VOICE)->getOutputScope(1))
    , synthScopeXY(processor.getProcessor(SynthGraphType::VOICE)->getOutputScope(0), processor.getProcessor(SynthGraphType::VOICE)->getOutputScope(1))
    , effectScopeL(processor.getProcessor(SynthGraphType::EFFECT)->getOutputScope(0))
    , effectScopeR(processor.getProcessor(SynthGraphType::EFFECT)->getOutputScope(1))
    , effectScopeXY(processor.getProcessor(SynthGraphType::EFFECT)->getOutputScope(0), processor.getProcessor(SynthGraphType::EFFECT)->getOutputScope(1))

    , mainFrame(
        [this](int i, const string& n) {
            if (!inited) return; // hack
            if(n == "scopes") guiUpdateTimer.startTimer((int)(1.f / 30.f* 1000.f));
            else guiUpdateTimer.stopTimer();
#if INTERCEPT_STD_STREAMS
            if(n == "debug") debugViewUpdateTimer->startTimer(1000);
            else debugViewUpdateTimer->stopTimer();
#endif
            if (n == "parameters") parameterEditor.startTimer();
            else parameterEditor.stopTimer();
        }
    )

    , settingsEditor(
        processor.subSettings
    )

    , voiceEditor(
        [this] { 
            auto d = Doc();
            processor.sdkExtensionManager.updateDoc(&d);
            return d;
        },
        processor.getPropagator(SynthGraphType::VOICE),
        processor.subComponentRegister,
        c_voiceChainInBus,
        c_voiceChainOutBus,
        [this](const string& c, const map<string, ModulePosition>& l) { processor.updateLayout(SynthGraphType::VOICE, c, l); }
    )

    , effectEditor(
        [this] {
            auto d = Doc();
            processor.sdkExtensionManager.updateDoc(&d);
            return d;
        },
        processor.getPropagator(SynthGraphType::EFFECT),
        processor.subComponentRegister,
        c_effectChainInBus,
        c_effectChainOutBus,
        [this](const string& c, const map<string, ModulePosition>& l) { processor.updateLayout(SynthGraphType::EFFECT, c, l); }
    )

#if INTERCEPT_STD_STREAMS
    , coutIntercept(std::cout)
    , cerrIntercept(std::cerr)
#endif

    , fileBrowserPanel(processor)

    , guiUpdateTimer(new function<void()>([this](){
        voiceScopeL.repaint();
        voiceScopeR.repaint();
        voiceScopeXY.repaint();
        synthScopeL.repaint();
        synthScopeR.repaint();
        synthScopeXY.repaint();
        effectScopeL.repaint();
        effectScopeR.repaint();
        effectScopeXY.repaint();
    }))
{

    setResizeLimits(128, 128, 8000, 8000);
    setConstrainer(nullptr);
    setResizable(true, true);
    addAndMakeVisible(mainFrame);
    
    mainFrame.addTab("scopes", Colours::black, &scopePPGrid, false);

    voiceScopeGrid.addComponent(&voiceScopeL);
    voiceScopeGrid.addComponent(&voiceScopeXY);
    voiceScopeGrid.addComponent(&voiceScopeR);
    voiceScopeGrid.setColoumns({ 0.33f, 0.33f, 0.33f });
    voiceScopeGrid.setText("voice");

    synthScopeGrid.addComponent(&synthScopeL);
    synthScopeGrid.addComponent(&synthScopeXY);
    synthScopeGrid.addComponent(&synthScopeR);
    synthScopeGrid.setColoumns({ 0.33f, 0.33f, 0.33f });
    synthScopeGrid.setText("synth");

    effectScopeGrid.addComponent(&effectScopeL);
    effectScopeGrid.addComponent(&effectScopeXY);
    effectScopeGrid.addComponent(&effectScopeR);
    effectScopeGrid.setColoumns({0.33f, 0.33f, 0.33f});
    effectScopeGrid.setText("effect");

    scopePPGrid.setColoumns({ 1.0f });
    scopePPGrid.addComponent(&voiceScopeGrid);
    scopePPGrid.addComponent(&synthScopeGrid);
    scopePPGrid.addComponent(&effectScopeGrid);

    mainFrame.addTab("parameters", Colours::black, &parameterEditor, false);
    processor.parameters.initializeKnobs(parameterEditor);

    mainFrame.addTab("settings", Colours::black, &settingsEditor, false);

    mainFrame.addTab("voice patch", Colours::black, &voiceEditor, false);
    mainFrame.addTab("effect patch", Colours::black, &effectEditor, false);

    mainFrame.addTab("files", Colours::black, &fileBrowserPanel, false);

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
#endif

    processor.broadcastPatch();

    setLookAndFeel(&processor.lookAndFeel);

    resized();

    // triple hack
    inited = true;
    mainFrame.setCurrentTabIndex(1);
    mainFrame.setCurrentTabIndex(0);

}

PhasePhckrEditor::~PhasePhckrEditor() {
    guiUpdateTimer.stopTimer();
#if INTERCEPT_STD_STREAMS
    debugViewUpdateTimer->stopTimer();
    delete debugViewUpdateTimer;
#endif
}

void PhasePhckrEditor::paint(Graphics& g) {
    g.fillAll(Colour(0xff111111));
}

void PhasePhckrEditor::resized() {
    mainFrame.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    repaint();
    processor.forceStateBump();
}
