#pragma once

#include <math.h>
#include <atomic>
#include <deque>
#include <iostream>
#include <set>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <climits>

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "GraphEditorModel.hpp"
#include "GraphEditorPositions.hpp"

#include "Utils.hpp"

using namespace std;

class PatchEditor;

class GraphEditor : public Component, public DragAndDropTarget
{
public:
    GraphEditor(
        PatchEditor& patchEditor,
        Viewport& viewPort,
        SubValue<Doc> &subDoc,
        SubValue<PatchDescriptor> &subPatch,
        const vector<PadDescription> &inBus,
        const vector<PadDescription> &outBus
    );
    ~GraphEditor();

    virtual void mouseDown(const MouseEvent & event) override;
    virtual void mouseDoubleClick(const MouseEvent & event) override;
    virtual void mouseDrag(const MouseEvent & event) override;
    virtual void mouseUp(const MouseEvent & event) override;
    virtual void mouseMove(const MouseEvent & event) override;
    virtual void mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel) override;

    virtual bool isInterestedInDragSource (const SourceDetails &dragSourceDetails) override {
        return true; // why not
    }
    virtual void itemDropped(const SourceDetails & dragSourceDetails) override;

    void paint (Graphics& g) override;

private:
    void updateBounds(const pair<XY, XY>& rectange);
    void updateBounds(const XY & position, const XY & size);

    void setGraph(const PatchDescriptor& patch);
    void propagateUserModelChange();

    void setDoc(const Doc& newDoc);

    void updateRenderComponents();

    // stuff migrated from GfxGraph
    vector<GfxModule> modules;
    list<GfxWire> wires;

    pair<XY, XY> getVirtualBounds();
    bool connect(const GfxLooseWire &looseWire, const XY &mousePos);
    bool disconnect(const XY& mousePos, GfxLooseWire &looseWire);
    void createComponentFromSelection(const set<string> & selectedModules, Doc & doc, XY& position);
    void moveDelta(XY delta);
    void moveIntoView();
    void designPorts(const Doc &doc);
    void recalculateWires(const vector<GfxModule>& modules);
    // end

    bool patchIsDirty;
    int subPatchHandle;
    PatchDescriptor patch;
    SubValue<PatchDescriptor> &subPatch;

    bool docIsDirty;
    Doc doc;
    SubValue<Doc> &subDoc;
    int docHandle;

    PatchEditor& patchEditor;
    Viewport& viewPort;

    vector<PadDescription> inBus;
    vector<PadDescription> outBus;

    simple_lock gfxGraphLock;

    GfxModule *draggedModule = nullptr;
    GfxLooseWire looseWire;
    bool mouseIsHovering = false;

    bool selecting;
    Point<float> selectionStart;
    Point<float> selectionStop;
    set<const GfxModule*> selectedModules;

    XY mouseDownPos;

    const float defaultZoom = 1.0f;
    const float zoomIncrement = 1.05f;

    float zoom;

    void clearZoom();
    void increaseZoom();
    void decreaseZoom();
    void applyZoom();

    void findHoverDoodat(const XY& pos);

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
        SubValue<Doc> &subDoc,
        SubValue<PatchDescriptor> & subscribedCGD,
        const vector<PadDescription> &inBus,
        const vector<PadDescription> &outBus
    );
    void paint(Graphics& g);
    void resized();
};
