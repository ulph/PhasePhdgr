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


struct UserAction {
    enum {
        none,
        mouse_down,
        mouse_double,
        mouse_up,
        mouse_drag,
        mouse_move
    };
    XY position;
};


class GraphView : public Component
{

public:
    GraphView(
        Viewport& parent,
        const Doc& doc,
        SubValue<ConnectionGraphDescriptor> * subscribedCGD,
        ModuleVariable inBus,
        ModuleVariable outBus
    )
        : subscribedCGD(subscribedCGD)
        , parent(parent)
        , doc(doc)
        , inBus(inBus)
        , outBus(outBus)
    {
        setBoundsRelative(0, 0, 1, 1);
        parent.setScrollOnDragEnabled(true);
        subscribedCGDhandle = subscribedCGD->subscribe(
            [this](const PhasePhckr::ConnectionGraphDescriptor& g){
                setGraph(g);
            }
        );
    }
    ~GraphView() {
        subscribedCGD->unsubscribe(subscribedCGDhandle);
    }
    virtual void mouseDown(const MouseEvent & event) override;
    virtual void mouseDrag(const MouseEvent & event) override;
    virtual void mouseUp(const MouseEvent & event) override;
    virtual void mouseMove(const MouseEvent & event) override;
    void paint (Graphics& g);
    void mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & d) override;

private:
    void updateBounds(const std::pair<XY, XY>& rectange);
    void updateBounds(const XY & position, const XY & size);

    void setGraph(const ConnectionGraphDescriptor& graph);
    void prepareRenderComponents(); // call _only_ from setGraph

    // make a copy data structures before calling
    void updateRenderComponents(
        const ConnectionGraphDescriptor & cgd,
        const std::map<std::string, XY> & mp
    );

    int subscribedCGDhandle;
    SubValue<ConnectionGraphDescriptor> * subscribedCGD;

    Viewport& parent;

    XY lastMouse;

    const ModuleVariable inBus;
    const ModuleVariable outBus;
    const Doc doc;

    std::atomic_flag userActionLock = ATOMIC_FLAG_INIT;
    GfxGraph gfxGraph_userActionCopy;

    std::atomic_flag connectionGraphDescriptorLock = ATOMIC_FLAG_INIT;
    ConnectionGraphDescriptor connectionGraphDescriptor;

    std::atomic_flag modulePositionsLock = ATOMIC_FLAG_INIT;
    std::map<std::string, XY> modulePositions;

    std::atomic_flag renderLock = ATOMIC_FLAG_INIT;
    GfxLooseWire * looseWire = nullptr;
    GfxGraph gfxGraph_renderCopy;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphView)
};

#endif  // GRAPHVIEW_H_INCLUDED