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
#include <functional>

#include <phasephdgr.hpp>

#include "GraphEditorModel.hpp"
#include "GraphEditorPositions.hpp"

#include "Utils.hpp"

#include "PopupMenus.hpp"

using namespace std;

class PatchEditor;

typedef function< void(const string&, const map<string, ModulePosition>&) > LayoutUpdateCallback;

class GraphEditor : public Component, public DragAndDropTarget
{
public:
    GraphEditor(
        PatchEditor& patchEditor,
        const string& rootComponent,
        const PatchDescriptor& initialPatch,
        Viewport& viewPort,
        const Doc &initialDoc,
        SubValue<PatchDescriptor> &subPatch,
        const vector<PadDescription> &inBus,
        const vector<PadDescription> &outBus,
        const LayoutUpdateCallback &layoutUpdateCb,
        bool readOnly
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

    void setDoc(const Doc& newDoc);
    void setGlobalComponents(const map<string, ComponentDescriptor>& globalComponents);

    void clearZoom();
    void increaseZoom();
    void decreaseZoom();

private:
    bool readOnly = false;
    void updateBounds(const pair<XY, XY>& rectange);
    void updateBounds(const XY & position, const XY & size);

    void setGraph(const PatchDescriptor& patch);
    void updateLayout();
    void propagatePatch();
    void propagateLayout();

    void updateRenderComponents();

    PortPopupMenuData portPopupMenuData;
    void portPopUpMenu(GfxModule & module, const string & port, bool inputPort);

    ModulePopupMenuData modulePopupMenuData;
    void modulePopUpMenu(bool validModule, const string& moduleName, const string& moduleType);

    void selectionPopUpMenu();

    vector<GfxModule> modules;
    list<GfxWire> wires;

    pair<XY, XY> getVirtualBounds();
    bool connect(GfxModule* module, GfxPort* port);
    bool autoConnect(const string &source, const string &target);
    bool disconnect(GfxWire* wire, const XY& mousePos, bool nearestSource);
    void moveDelta(XY delta);
    void moveIntoView();
    void designPorts(const Doc &doc);
    void recalculateWires(vector<GfxModule>& modules);

    ComponentDescriptor* rootComponent();

    const string rootComponentName = "";
    bool patchIsDirty;
    int subPatchHandle;
    PatchDescriptor patch;
    SubValue<PatchDescriptor> &subPatch;

    map<string, ComponentDescriptor> globalComponents;
    bool docIsDirty;
    Doc doc;

    PatchEditor& patchEditor;
    Viewport& viewPort;

    vector<PadDescription> inBus;
    vector<PadDescription> outBus;

    simple_lock gfxGraphLock;

    GfxModule *dragModule = nullptr;
    XY dragDistance;
    GfxLooseWire looseWire;
    bool mouseIsHovering = false;

    bool selecting;
    Point<float> selectionStart;
    Point<float> selectionStop;
    set<const GfxModule*> selectedModules;

    XY mouseDownPos;

    const float defaultZoom = 0.9f;
    const float zoomIncrement = 1.05f;

    float zoom;

    void applyZoom();

    void findCloseThings(const XY& pos, GfxPort** closestPort, GfxModule** closestModule, GfxWire** closestWire, bool& nearestSource);
    void findHoverDoodat(const XY& pos);

    LayoutUpdateCallback layoutUpdateCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphEditor)
};


class GraphViewPort : public Viewport{
public:
    GraphViewPort() : Viewport("...") {
        setScrollOnDragEnabled(true);
        setScrollBarsShown(true, true, true, true);
    }
    void paint(Graphics& g){
        g.fillAll(Colours::black);
    }
};


class GraphEditorBundle : public Component, public Button::Listener {
private:
    GraphViewPort view;
public:
    GraphEditorBundle(
        PatchEditor& graphEditor,
        const Doc &initialDoc,
        SubValue<PatchDescriptor> & subscribedCGD,
        const string& rootComponent,
        const PatchDescriptor& initialPatch,
        const vector<PadDescription> &inBus,
        const vector<PadDescription> &outBus,
        const LayoutUpdateCallback &layoutUpdateCb,
        bool readOnly
    );
    GraphEditor editor;

    TextButton decreaseZoomButton;
    TextButton resetZoomButton;
    TextButton increaseZoomButton;

    void paint(Graphics& g) override;
    void resized() override;

    void buttonClicked(Button* btn) override;

};
