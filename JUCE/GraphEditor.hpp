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


class GraphEditor : public Component
{
    Doc doc;
    PhasePhckrGrid grid;
    PhasePhckrGrid docGrid;
    GraphViewPort viewPort;
    GraphView graphView;
    ConnectionGraphTextEditor textEditor;
    TextEditor docView;
    ListBox docList;
    DocListModel docListModel;
    ModuleVariable inBus;
    ModuleVariable outBus;
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
        , graphView(
            viewPort,
            doc,
            subscribedCGD,
            inBus,
            outBus
        )
    {
        viewPort.setViewedComponent(&graphView, false);
        addAndMakeVisible(grid);
        grid.addComponent(&textEditor);
        grid.addComponent(&viewPort);
        grid.addComponent(&docGrid);
        grid.setColoumns({0.25f, 0.5f, 0.25f});
        docGrid.addComponent(&docView);
        docGrid.addComponent(&docList);
        docGrid.setColoumns({1.0f});
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