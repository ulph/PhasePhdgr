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
    void updateBounds();

    void setGraph(const ConnectionGraphDescriptor& graph);
    void prepareRenderComponents();
    void createRenderComponents(const std::map<std::string, XY> &modulePositions);

    int subscribedCGDhandle;
    SubValue<ConnectionGraphDescriptor> * subscribedCGD;

    Viewport& parent;

    XY lastMouse;

    const ModuleVariable inBus;
    const ModuleVariable outBus;
    const Doc doc;

    std::atomic_flag dataLock = ATOMIC_FLAG_INIT;
    ConnectionGraphDescriptor connectionGraphDescriptor;

    std::atomic_flag renderLock = ATOMIC_FLAG_INIT;
    GfxLooseWire* looseWire = nullptr;
    GfxModule * movingModule = nullptr;
    GfxGraph gfxGraph;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphView)
};

#endif  // GRAPHVIEW_H_INCLUDED