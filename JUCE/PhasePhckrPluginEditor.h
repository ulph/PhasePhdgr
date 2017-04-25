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

struct LambdaTimer : Timer {
    LambdaTimer(std::function<void()> *callBack) {
        cb = callBack;
    }
    virtual void timerCallback() {
        (*cb)();
    }
    virtual ~LambdaTimer() {
        delete cb;
    }
private:
    const std::function<void(void)> *cb;
};


class DocListModel : public ListBoxModel {
private:
    std::map<std::string, ModuleDoc> moduleDocs;
    std::vector<std::string> rows;
    TextEditor & docView;
public:
    DocListModel(const std::map<std::string, ModuleDoc> & moduleDocs, TextEditor & docView)
        : ListBoxModel()
        , moduleDocs(moduleDocs)
        , docView(docView)
    {
        for (const auto &kv : moduleDocs) {
            rows.push_back(kv.first);
        }
    }
    virtual int getNumRows() {
        return rows.size();
    }
    virtual void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) {
        const auto &key = rows[rowNumber];
        const auto &doc = moduleDocs.at(key);
        g.drawFittedText(doc.type, 0, 0, width, height, Justification::centred, 1);
    }
    virtual void listBoxItemClicked(int row, const MouseEvent &) {
        if (row >= 0) {
            const auto &key = rows[row];
            const auto &doc = moduleDocs[key];
            docView.clear();
            docView.insertTextAtCaret(doc.type + "\n\n");
            docView.insertTextAtCaret("inputs:\n");
            for (const auto & i : doc.inputs) {
                docView.insertTextAtCaret("  " + i.name + ((i.unit != "") ? (" [" + i.unit + "]") : "") + " " + std::to_string(i.value) + "\n");
            }
            docView.insertTextAtCaret("\noutputs:\n");
            for (const auto & o : doc.outputs) {
                docView.insertTextAtCaret("  " + o.name + ((o.unit != "") ? (" [" + o.unit + "]") : "") + "\n");
            }
            docView.insertTextAtCaret("\n\n" + doc.docString);
            docView.moveCaretToTop(false);
        }
    }
};


class ConnectionGraphTextEditor : public TextEditor {
private:
    int handle;
    SubValue<PhasePhckr::ConnectionGraphDescriptor> * sub;
public:
    ConnectionGraphTextEditor(SubValue<PhasePhckr::ConnectionGraphDescriptor> * sub)
        : TextEditor()
        , sub(sub)
    {
        setMultiLine(true, true);
        handle = sub->subscribe(
            [this](const PhasePhckr::ConnectionGraphDescriptor g) {
            setText(json(g).dump(2));
        }
        );
    }
    ~ConnectionGraphTextEditor() {
        sub->unsubscribe(handle);
    }
};


class GraphViewPort : public Viewport {
public:
    GraphViewPort() : Viewport("...") {}
    void mouseWheelMove(const MouseEvent &, const MouseWheelDetails &) override {}
};


class PhasePhckrAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    PhasePhckrAudioProcessorEditor (
        PhasePhckrAudioProcessor&,
        SubValue<PhasePhckr::ConnectionGraphDescriptor> &activeVoice,
        SubValue<PhasePhckr::ConnectionGraphDescriptor> &activeEffect
     );
    ~PhasePhckrAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    PhasePhckrAudioProcessor& processor;

    PhasePhckr::Doc voiceDoc;
    PhasePhckr::Doc effectDoc;

    SubValue<PhasePhckr::ConnectionGraphDescriptor> &activeVoice;
    SubValue<PhasePhckr::ConnectionGraphDescriptor> &activeEffect;
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
    PhasePhckrGrid editorGrid;

    ConnectionGraphTextEditor voiceEditor;
    ConnectionGraphTextEditor effectEditor;
    PhasePhckrGrid editorMenu;

    PhasePhckrGrid performGrid;

    DirectoryContentsList voiceDirectoryWatcher;
    FileListComponent voiceDirectoryList;
    PhasePhckrFileStuff::StupidFileBrowserListener voiceListListener;

    DirectoryContentsList effectDirectoryWatcher;
    FileListComponent effectDirectoryList;
    PhasePhckrFileStuff::StupidFileBrowserListener effectListListener;

    GraphViewPort voiceGraphViewport;
    GraphViewPort effectGraphViewport;
    GraphView voiceGraphView;    
    GraphView effectGraphView;

    // TODO, bundle?
    PhasePhckr::Doc doc;
    TextEditor docView;
    ListBox docList;
    DocListModel docListModel;

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
