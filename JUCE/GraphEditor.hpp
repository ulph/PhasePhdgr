#ifndef GRAPHVIEW_H_INCLUDED
#define GRAPHVIEW_H_INCLUDED

#include "GraphEditorModel.hpp"
#include "GraphEditorPositions.hpp"

#include "JuceLibraryCode/JuceHeader.h"
#include "design.hpp"
#include <iostream>
#include <set>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <climits>
#include "docs.hpp"
#include "Utils.hpp"
#include <math.h>
#include <atomic>
#include <deque>

using namespace std;

class PatchEditor;

class GraphEditor : public Component, public DragAndDropTarget
{
public:
    GraphEditor(
        PatchEditor& patchEditor,
        Viewport& viewPort,
        const Doc& doc,
        SubValue<PatchDescriptor> & subPatch,
        const vector<PadDescription> &inBus,
        const vector<PadDescription> &outBus
    );
    ~GraphEditor();
    virtual void mouseDown(const MouseEvent & event) override;
    virtual void mouseDoubleClick(const MouseEvent & event) override;
    virtual void mouseDrag(const MouseEvent & event) override;
    virtual void mouseUp(const MouseEvent & event) override;
    virtual void mouseMove(const MouseEvent & event) override;
    virtual bool isInterestedInDragSource (const SourceDetails &dragSourceDetails){
        return true; // why not
    }
    virtual void itemDropped(const SourceDetails & dragSourceDetails) override;

    void paint (Graphics& g);
    void mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & d) override;

private:
    void updateBounds(const pair<XY, XY>& rectange);
    void updateBounds(const XY & position, const XY & size);

    void setGraph(const PatchDescriptor& graph);
    void propagateUserModelChange();

    // make a copy data structures before calling
    void updateRenderComponents(
        const PatchDescriptor & cgd,
        const ModulePositionMap & mp
    );

    int subPatchHandle;
    SubValue<PatchDescriptor> & subPatch;

    PatchEditor& patchEditor;
    Viewport& viewPort;

    vector<PadDescription> inBus;
    vector<PadDescription> outBus;
    Doc doc;

    atomic_flag gfxGraphLock = ATOMIC_FLAG_INIT;
    GfxGraph gfxGraph;
    GfxModule * draggedModule = nullptr;
    GfxLooseWire looseWire;

    Point<float> selectionStart;
    Point<float> selectionStop;
    bool selecting;
    set<const GfxModule*> selectedModules;

    XY mouseDownPos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphEditor)
};


class GraphViewPort : public Viewport{
public:
    GraphViewPort() : Viewport("...") {
        setScrollOnDragEnabled(true);
        setScrollBarsShown(true, true, true, true);
    }
};


class GraphEditorBundle : public Component{
private:
    GraphViewPort viewPort;
    GraphEditor graphView;
public:
    GraphEditorBundle(
        PatchEditor& graphEditor,
        const Doc& doc,
        SubValue<PatchDescriptor> & subscribedCGD,
        const vector<PadDescription> &inBus,
        const vector<PadDescription> &outBus
    );
    void paint(Graphics& g);
    void resized();
};


#endif  // GRAPHVIEW_H_INCLUDED
