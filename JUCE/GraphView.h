#ifndef GRAPHVIEW_H_INCLUDED
#define GRAPHVIEW_H_INCLUDED

#include "GraphViewGfx.hpp"

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
#include "GraphViewNodeStuff.hpp"

class GraphEditor;

class GraphView : public Component, public DragAndDropTarget
{
public:
    GraphView(
        GraphEditor& graphEditor,
        Viewport& viewPort,
        const Doc& doc,
        SubValue<ConnectionGraphDescriptor> & subscribedCGD,
        const ModuleVariable& inBus,
        const ModuleVariable& outBus
    )
        : subscribedCGD(subscribedCGD)
        , graphEditor(graphEditor)
        , viewPort(viewPort)
        , doc(doc)
        , inBus(inBus)
        , outBus(outBus)
        , scale(1.0f)
    {
        viewPort.setScrollOnDragEnabled(true);
        subscribedCGDhandle = subscribedCGD.subscribe(
            [this](const PhasePhckr::ConnectionGraphDescriptor& g){
                setGraph(g);
            }
        );
    }
    ~GraphView() {
        subscribedCGD.unsubscribe(subscribedCGDhandle);
    }
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
    float scale;
    void updateBounds(const pair<XY, XY>& rectange);
    void updateBounds(const XY & position, const XY & size);

    void setGraph(const ConnectionGraphDescriptor& graph);
    void propagateUserModelChange();
    atomic_flag connectionGraphDescriptorLock = ATOMIC_FLAG_INIT;

    // make a copy data structures before calling
    void updateRenderComponents(
        const ConnectionGraphDescriptor & cgd,
        const ModulePositionMap & mp
    );

    int subscribedCGDhandle;
    SubValue<ConnectionGraphDescriptor> & subscribedCGD;

    GraphEditor& graphEditor;
    Viewport& viewPort;

    ModuleVariable inBus;
    ModuleVariable outBus;
    Doc doc;

    atomic_flag gfxGraphLock = ATOMIC_FLAG_INIT;
    GfxGraph gfxGraph;
    GfxModule * draggedModule = nullptr;
    GfxLooseWire looseWire;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphView)
};

#endif  // GRAPHVIEW_H_INCLUDED
