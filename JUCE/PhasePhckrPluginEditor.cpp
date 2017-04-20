#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrPluginEditor.h"
#include "PhasePhckr.h"
#include <cstring>
#include "DirectoryWatcher.hpp"
#include "Style.hpp"

PhasePhckrLookAndFeel g_lookAndFeel;

using namespace PhasePhckrFileStuff;

const Font monoFont = Font(Font::getDefaultMonospacedFontName(), 14, 0);

static void configureEditor(TextEditor& ed) {
    ed.setMultiLine(true, false);
}

static void configureFileList(FileListComponent& bx) {
    //
}

static void configureTabList(TabbedComponent& tbc) {
    //
}

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
    , voiceDirectoryWatcher(PhasePhckrFileStuff::getFilter(), PhasePhckrFileStuff::getThread())
    , effectDirectoryWatcher(PhasePhckrFileStuff::getFilter(), PhasePhckrFileStuff::getThread())
    , voiceDirectoryList(voiceDirectoryWatcher)
    , effectDirectoryList(effectDirectoryWatcher)
    , voiceListListener([this](const File& f) { processor.setVoicePatch(loadJson(f)); voiceEditor.setText(PhasePhckr::prettydump(processor.getVoicePatch())); })
    , effectListListener([this](const File& f) { processor.setEffectPatch(loadJson(f)); effectEditor.setText(PhasePhckr::prettydump(processor.getEffectPatch())); })
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
    scopeGrid.setNumberOfColumns(3);

    mainFrame.addTab("files", g_tabColor, &editorGrid, false);
    editorGrid.addComponent(&voiceEditor);
    editorGrid.addComponent(&editorMenu);
    editorGrid.addComponent(&effectEditor);
    editorGrid.setNumberOfColumns(3);
    editorMenu.addComponent(&voiceDirectoryList);
    editorMenu.addComponent(&effectDirectoryList);
    editorMenu.setNumberOfColumns(2);

    mainFrame.addTab("view", g_tabColor, nullptr, false);
    // TODO, something here

    configureTabList(mainFrame);

    configureFileList(voiceDirectoryList);
    configureFileList(effectDirectoryList);

    voiceDirectoryList.addListener(&voiceListListener);
    effectDirectoryList.addListener(&effectListListener);

    configureEditor(voiceEditor);
    configureEditor(effectEditor);

    voiceDirectoryWatcher.setDirectory(PhasePhckrFileStuff::voicesDir, true, true);
    effectDirectoryWatcher.setDirectory(PhasePhckrFileStuff::effectsDir, true, true);

    voiceEditor.setText(PhasePhckr::prettydump(processor.getVoicePatch()));
    effectEditor.setText(PhasePhckr::prettydump(processor.getEffectPatch()));

    resized();
}

PhasePhckrAudioProcessorEditor::~PhasePhckrAudioProcessorEditor()
{
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
