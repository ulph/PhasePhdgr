#pragma once

#include "JuceLibraryCode/JuceHeader.h"
#include "PhasePhckrGrid.h"
#include <map>
#include <vector>
#include "docs.hpp"
#include "design.hpp"
#include "Utils.hpp"
#include "GraphView.h"

using namespace PhasePhckr;
using namespace std;

void _stylize(TextEditor* t);
void _stylize(ListBox* l);
void _stylize(FileListComponent* l);

typedef SubValue<PatchDescriptor> SubPatch;

class DocListModel : public ListBoxModel {
private:
    map<string, ModuleDoc> moduleDocs;
    vector<string> rows;
    TextEditor & docView;
public:
    DocListModel(const map<string, ModuleDoc> & moduleDocs, TextEditor & docView);
    void setDocs(const map<string, ModuleDoc> & moduleDocs);
    virtual int getNumRows();
    virtual void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected);
    virtual void listBoxItemClicked(int row, const MouseEvent &);
    virtual var getDragSourceDescription (const SparseSet< int > &rowsToDescribe){
        if(!rowsToDescribe.isEmpty()){
            return String(rows[rowsToDescribe[0]]);
        }
        return var();
    }
};


class ConnectionGraphTextEditor : public TextEditor {
private:
    int handle;
    SubPatch & sub;
public:
    ConnectionGraphTextEditor(SubPatch & sub);
    virtual ~ConnectionGraphTextEditor();
};


class GraphViewPort : public Viewport{
public:
    GraphViewPort() : Viewport("...") {}
    void mouseWheelMove(const MouseEvent &, const MouseWheelDetails &) override {}
};


class GraphViewBundle : public Component{
private:
    GraphViewPort viewPort;
    GraphView graphView;
    PhasePhckrGrid bottomRow;
    TextButton resetLayoutButton;
    Label fileName;
    TextButton saveButton;
public:
    GraphViewBundle(
        GraphEditor& graphEditor,
        const Doc& doc,
        SubPatch & subscribedCGD,
        const ModuleVariable& inBus,
        const ModuleVariable& outBus
    );
    void paint(Graphics& g);
    void resized();
};


class GraphEditorTabbedComponent : public TabbedComponent {
private:
    list<pair<int, SubPatch>> & cdgs;
public:
    GraphEditorTabbedComponent(list<pair<int, SubPatch>> & cdgs)
        : TabbedComponent(TabbedButtonBar::TabsAtTop)
        , cdgs(cdgs)
    {
    }
    virtual void currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName) {
        while (newCurrentTabIndex + 1 != getNumTabs()) {
            removeTab(getNumTabs() - 1);
            cdgs.back().second.unsubscribe(cdgs.back().first);
            cdgs.pop_back();
        }
    }
};


class GraphEditor : public Component
{
    Doc doc;
    SubPatch & patch;
    PatchDescriptor patchCopy;
    int patchHandle;

    PhasePhckrGrid grid;
    PhasePhckrGrid docGrid;

    GraphViewBundle rootView;

    ConnectionGraphTextEditor textEditor;
    TextEditor docView;
    ListBox docList;
    DocListModel docListModel;

    ModuleVariable inBus;
    ModuleVariable outBus;

    list<pair<int, SubPatch>> componentGraphs;
    GraphEditorTabbedComponent editorStack;
    friend class GraphView;
    void push_tab(const string& componentName, const string& componentType);

public:
    GraphEditor(
        const Doc &doc,
        SubPatch &subscribedCGD,
        const ModuleVariable &inBus,
        const ModuleVariable &outBus
    );
    virtual ~GraphEditor();
    void paint(Graphics& g);
    void resized();
};
