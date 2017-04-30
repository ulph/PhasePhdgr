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
        return (int)rows.size();
    }
    virtual void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) {
        g.setColour(Colours::black);
        g.fillAll();
        g.setColour(Colours::green);
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
    SubValue<PhasePhckr::ConnectionGraphDescriptor> & sub;
public:
    ConnectionGraphTextEditor(SubValue<PhasePhckr::ConnectionGraphDescriptor> & sub)
        : TextEditor()
        , sub(sub)
    {
        setMultiLine(true, true);
        handle = sub.subscribe(
            [this](const PhasePhckr::ConnectionGraphDescriptor g) {
            setText(json(g).dump(2));
        }
        );
        setColour(TextEditor::backgroundColourId, Colours::black);
        setColour(TextEditor::textColourId, Colours::green);
        setColour(TextEditor::highlightColourId, Colours::darkgreen);
        setColour(TextEditor::highlightedTextColourId, Colours::yellow);
        setColour(TextEditor::outlineColourId, Colours::black);
        setColour(TextEditor::focusedOutlineColourId, Colours::black);
        setColour(TextEditor::shadowColourId, Colours::black);
    }
    ~ConnectionGraphTextEditor() {
        sub.unsubscribe(handle);
    }
};


class GraphViewPort : public Viewport {
public:
    GraphViewPort() : Viewport("...") {}
    void mouseWheelMove(const MouseEvent &, const MouseWheelDetails &) override {}
};

class GraphViewBundle : public Component {
    GraphViewPort viewPort;
    GraphView graphView;
public:
    GraphViewBundle(
        GraphEditor& graphEditor,
        const Doc& doc,
        SubValue<ConnectionGraphDescriptor> & subscribedCGD,
        const ModuleVariable& inBus,
        const ModuleVariable& outBus
    )
    : graphView(
        graphEditor
        , viewPort
        , doc
        , subscribedCGD
        , inBus
        , outBus
    )
    {
        addAndMakeVisible(viewPort);
        viewPort.setViewedComponent(&graphView, false);
        resized();
    }
    void paint(Graphics& g)
    {
        g.fillAll(Colours::black);
    }
    void resized()
    {
        viewPort.setBoundsRelative(0, 0, 1, 1);
        repaint();
    }
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

    void push_tab(const string& componentName, const string& componentType) {
        return;

        // WIP - a graph view should be able to request a new tab for a _component_
        // a bunch of mocking needs to be done, should be similar to what's 
        // done for the root, so perhaps it can be conceptualized
        const auto& d = doc.get();
        auto dit = d.find(componentType);
        if (dit != d.end()) {
            assert(componentType == dit->second.type);
            auto componentGraph = SubValue<PhasePhckr::ConnectionGraphDescriptor>();
            // TODO, mock bus modules types - inBus shows component inputs as outputs ... outBus vice versa
            // TODO, mock doc (inject the new module types)
            // TODO, build graph - needs to _flatten_ before passing to design.cpp
            ModuleVariable inBus = {componentName +" inBus", componentType };
            ModuleVariable outBus = {componentName +" outBus", componentType };
            editorStack.addTab(
                componentName + " (" + componentType + ")",
                Colours::black, new GraphViewBundle(
                    *this,
                    doc,
                    componentGraph,
                    inBus,
                    outBus
                ),
                true
            );
            componentGraphs.emplace_back(componentGraph);
        }
    }

public:
    GraphEditor(
        const Doc &doc,
        SubValue<PhasePhckr::ConnectionGraphDescriptor> &subscribedCGD,
        const ModuleVariable &inBus,
        const ModuleVariable &outBus
    )
        : doc(doc)
        , docListModel(doc.get(), docView)
        , docList("docList", &docListModel)
        , inBus(inBus)
        , outBus(outBus)
        , textEditor(subscribedCGD)
        , rootView(
            *this,
            doc,
            subscribedCGD,
            inBus,
            outBus
        )
        , editorStack(TabbedButtonBar::TabsAtTop)
    {
        addAndMakeVisible(grid);
        grid.addComponent(&textEditor);
        grid.addComponent(&editorStack);
        grid.addComponent(&docGrid);
        grid.setColoumns({0.125f, 0.75f, 0.125f});

        editorStack.addTab("root", Colours::black, &rootView, false);

        docGrid.addComponent(&docView);
        docGrid.addComponent(&docList);
        docGrid.setColoumns({1.0f});
        docList.updateContent();
        docView.setMultiLine(true, true);
        docView.setColour(TextEditor::backgroundColourId, Colours::black);
        docView.setColour(TextEditor::textColourId, Colours::green);
        docView.setColour(TextEditor::highlightColourId, Colours::darkgreen);
        docView.setColour(TextEditor::highlightedTextColourId, Colours::yellow);
        docView.setColour(TextEditor::outlineColourId, Colours::black);
        docView.setColour(TextEditor::focusedOutlineColourId, Colours::black);
        docView.setColour(TextEditor::shadowColourId, Colours::black);
        resized();
    }

    void paint(Graphics& g)
    {
        g.fillAll(Colours::black);
    }

    void resized()
    {
        grid.setBoundsRelative(0, 0, 1, 1);
        repaint();
    }
};