#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "JuceLibraryCode/JuceHeader.h"
#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrScope.h"
#include "DirectoryWatcher.hpp"
#include "GraphView.h"
#include "docs.hpp"
#include <vector>

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

struct InterceptStringStream {
    InterceptStringStream(std::ostream & stream)
        : oldBuf(stream.rdbuf( newBuf.rdbuf() ) )
    { 
    }
    ~InterceptStringStream() {
        std::cout.rdbuf(oldBuf);
    }
    void readAll(std::string & target) {
        char ch = newBuf.rdbuf()->sbumpc();
        while (ch != EOF) {
            target += ch;
            ch = newBuf.rdbuf()->sbumpc();
        }
    }
private:
    std::streambuf * oldBuf;
    std::stringstream newBuf;
};

class DocListModel : public ListBoxModel {
private:
    std::vector<ModuleDoc> moduleDocs;
    TextEditor & docView;
public:
    DocListModel(const std::vector<ModuleDoc> & moduleDocs, TextEditor & docView)
        : ListBoxModel()
        , moduleDocs(moduleDocs)
        , docView(docView)
    {}
    virtual int getNumRows() {
        return moduleDocs.size();
    }
    virtual void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) {
        const auto & doc = moduleDocs.at(rowNumber);
        g.drawFittedText(doc.type, 0, 0, width, height, Justification::centred, 1);
    }
    virtual void listBoxItemClicked(int row, const MouseEvent &) {
        if (row >= 0) {
            const ModuleDoc &doc = moduleDocs[row];
            docView.clear();
            docView.insertTextAtCaret(doc.type + "\n\n");
            docView.insertTextAtCaret("inputs:\n");
            for (const auto & i : doc.inputs) {
                docView.insertTextAtCaret("  " + i.name + ((i.unit != "") ? (" [" + i.unit + "]") : "") + " " + std::to_string(i.value) + "\n");
            }
            docView.insertTextAtCaret("\noutputs:\n");
            for (const auto & o : doc.outputs) {
                docView.insertTextAtCaret("  " + o.name + ((o.unit != "") ? (" [" + o.unit + "]") : "") + " " + std::to_string(o.value) + "\n");
            }
            docView.insertTextAtCaret("\n\n" + doc.docString);
            docView.moveCaretToTop(false);
        }
    }
};

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

    PhasePhckr::Doc doc;
    TextEditor docView;
    ListBox docList;
    DocListModel docListModel;

    InterceptStringStream coutIntercept;
    InterceptStringStream cerrIntercept;
    TextEditor coutView;
    TextEditor cerrView;
    PhasePhckrGrid debugTab;
    LambdaTimer* debugViewUpdateTimer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
