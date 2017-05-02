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


class DocListModel : public ListBoxModel {
private:
    std::map<std::string, ModuleDoc> moduleDocs;
    std::vector<std::string> rows;
    TextEditor & docView;
public:
    DocListModel(const std::map<std::string, ModuleDoc> & moduleDocs, TextEditor & docView);
    virtual int getNumRows();
    virtual void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected);
    virtual void listBoxItemClicked(int row, const MouseEvent &);
    virtual var getDragSourceDescription (const SparseSet< int > &rowsToDescribe){
        if(!rowsToDescribe.isEmpty()){
            return String(rows[rowsToDescribe[0]]);
        }
    }
};


class ConnectionGraphTextEditor : public TextEditor {
private:
    int handle;
    SubValue<PhasePhckr::ConnectionGraphDescriptor> & sub;
public:
    ConnectionGraphTextEditor(SubValue<PhasePhckr::ConnectionGraphDescriptor> & sub);
    virtual ~ConnectionGraphTextEditor();
};


class GraphViewPort : public Viewport {
public:
    GraphViewPort() : Viewport("...") {}
    void mouseWheelMove(const MouseEvent &, const MouseWheelDetails &) override {}
};


class GraphViewBundle : public Component, public DragAndDropContainer {
    GraphViewPort viewPort;
    GraphView graphView;
public:
    GraphViewBundle(
        GraphEditor& graphEditor,
        const Doc& doc,
        SubValue<ConnectionGraphDescriptor> & subscribedCGD,
        const ModuleVariable& inBus,
        const ModuleVariable& outBus
    );
    void paint(Graphics& g);
    void resized();
};


class GraphEditor : public Component
{
    Doc doc;
    PhasePhckrGrid grid;
    PhasePhckrGrid docGrid;

    GraphViewBundle rootView;

    ConnectionGraphTextEditor textEditor;
    TextEditor docView;
    ListBox docList;
    DocListModel docListModel;

    ModuleVariable inBus;
    ModuleVariable outBus;

    vector<SubValue<PhasePhckr::ConnectionGraphDescriptor>> componentGraphs;
    TabbedComponent editorStack;
    friend class GraphView;

    void push_tab(const string& componentName, const string& componentType);

public:
    GraphEditor(
        const Doc &doc,
        SubValue<PhasePhckr::ConnectionGraphDescriptor> &subscribedCGD,
        const ModuleVariable &inBus,
        const ModuleVariable &outBus
    );
    void paint(Graphics& g);
    void resized();
};
