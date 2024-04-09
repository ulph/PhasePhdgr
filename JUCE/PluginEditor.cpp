#include <cstring>
#include <typeinfo>

#include <phasephdgr.hpp>
#include <phasephdgr_json.hpp>

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DirectoryWatcher.hpp"

#include "FileIO.hpp"

using namespace PhasePhdgrFileStuff;

using namespace std;

PhasePhdgrEditorBase::PhasePhdgrEditorBase(PhasePhdgrProcessorBase& p)
    : AudioProcessorEditor (&p), processor (p)
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
        for(auto &s: scopes) s->repaint();
    }))
{

    setResizeLimits(128, 128, 8000, 8000);
    setConstrainer(nullptr);
    setResizable(true, true);
    addAndMakeVisible(mainFrame);
    
    mainFrame.addTab("scopes", Colours::black, &scopePPGrid, false);

    scopePPGrid.setColoumns({ 1.0f });

    mainFrame.addTab("parameters", Colours::black, &parameterEditor, false);
    processor.parameters.initializeKnobs(parameterEditor);

    if(processor.isSynth()){
        mainFrame.addTab("settings", Colours::black, &settingsEditor, false);
        mainFrame.addTab("voice patch", Colours::black, &voiceEditor, false);
    }

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

PhasePhdgrEditorBase::~PhasePhdgrEditorBase() {
    guiUpdateTimer.stopTimer();
#if INTERCEPT_STD_STREAMS
    debugViewUpdateTimer->stopTimer();
    delete debugViewUpdateTimer;
#endif
    for(auto s: scopes) delete s;
}

void PhasePhdgrEditorBase::paint(Graphics& g) {
    g.fillAll(Colour(0xff111111));
}

void PhasePhdgrEditorBase::resized() {
    mainFrame.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    repaint();
    processor.forceStateBump();
}

PhasePhdgrEditor::PhasePhdgrEditor(PhasePhdgrProcessorBase& p) : PhasePhdgrEditorBase(p) {
    voiceScopeGrid.setText("voice");
    scopePPGrid.addComponent(&voiceScopeGrid);
    synthScopeOutGrid.setText("synth");
    scopePPGrid.addComponent(&synthScopeOutGrid);
    auto s = processor.getProcessor(SynthGraphType::VOICE);
    auto voiceScopeL = new ScopeView(s->getVoiceScope(0));
    auto voiceScopeR = new ScopeView(s->getVoiceScope(1));
    auto voiceScopeXY = new XYScopeView(s->getVoiceScope(0), s->getVoiceScope(1));
    scopes.push_back(voiceScopeL);
    scopes.push_back(voiceScopeR);
    scopes.push_back(voiceScopeXY);
    voiceScopeGrid.addComponent(voiceScopeL);
    voiceScopeGrid.addComponent(voiceScopeR);
    voiceScopeGrid.addComponent(voiceScopeXY);
    voiceScopeGrid.setColoumns({ 0.33f, 0.33f, 0.33f });

    auto synthScopeL = new ScopeView(s->getOutputScope(0));
    auto synthScopeR = new ScopeView(s->getOutputScope(1));
    auto synthScopeXY = new XYScopeView(s->getOutputScope(0), s->getOutputScope(1));
    scopes.push_back(synthScopeL);
    scopes.push_back(synthScopeR);
    scopes.push_back(synthScopeXY);
    synthScopeOutGrid.addComponent(synthScopeL);
    synthScopeOutGrid.addComponent(synthScopeR);
    synthScopeOutGrid.addComponent(synthScopeXY);
    synthScopeOutGrid.setColoumns({ 0.33f, 0.33f, 0.33f });

    effectScopeOutGrid.setText("effect");
    scopePPGrid.addComponent(&effectScopeOutGrid);
    auto e = processor.getProcessor(SynthGraphType::EFFECT);

    auto effectScopeL = new ScopeView(e->getOutputScope(0));
    auto effectScopeR = new ScopeView(e->getOutputScope(1));
    auto effectScopeXY = new XYScopeView(e->getOutputScope(0), e->getOutputScope(1));
    scopes.push_back(effectScopeL);
    scopes.push_back(effectScopeR);
    scopes.push_back(effectScopeXY);
    effectScopeOutGrid.addComponent(effectScopeL);
    effectScopeOutGrid.addComponent(effectScopeR);
    effectScopeOutGrid.addComponent(effectScopeXY);
    effectScopeOutGrid.setColoumns({ 0.33f, 0.33f, 0.33f });

}

PhasePhdgrEditorFX::PhasePhdgrEditorFX(PhasePhdgrProcessorBase& p) : PhasePhdgrEditorBase(p) {
    effectScopeInGrid.setText("input");
    scopePPGrid.addComponent(&effectScopeInGrid);

    effectScopeOutGrid.setText("output");
    scopePPGrid.addComponent(&effectScopeOutGrid);

    auto e = processor.getProcessor(SynthGraphType::EFFECT);
    if(!e) return;

    auto inputScopeL = new ScopeView(e->getInputScope(0));
    auto inputScopeR = new ScopeView(e->getInputScope(1));
    auto inputScopeXY = new XYScopeView(e->getInputScope(0), e->getInputScope(1));
    scopes.push_back(inputScopeL);
    scopes.push_back(inputScopeR);
    scopes.push_back(inputScopeXY);
    effectScopeInGrid.addComponent(inputScopeL);
    effectScopeInGrid.addComponent(inputScopeR);
    effectScopeInGrid.addComponent(inputScopeXY);
    effectScopeInGrid.setColoumns({ 0.33f, 0.33f, 0.33f });

    auto effectScopeL = new ScopeView(e->getOutputScope(0));
    auto effectScopeR = new ScopeView(e->getOutputScope(1));
    auto effectScopeXY = new XYScopeView(e->getOutputScope(0), e->getOutputScope(1));
    scopes.push_back(effectScopeL);
    scopes.push_back(effectScopeR);
    scopes.push_back(effectScopeXY);
    effectScopeOutGrid.addComponent(effectScopeL);
    effectScopeOutGrid.addComponent(effectScopeR);
    effectScopeOutGrid.addComponent(effectScopeXY);
    effectScopeOutGrid.setColoumns({ 0.33f, 0.33f, 0.33f });
}