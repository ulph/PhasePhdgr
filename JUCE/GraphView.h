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

class GraphView : public Component, public DragAndDropContainer
{
protected:
    virtual void dragOperationStarted (){

    }

    virtual void dragOperationEnded (){
        auto juceThing = getCurrentDragDescription().toString();
        auto thing = juceThing.toStdString();
        auto d = doc.get();
        auto mIt = d.find(thing);
        if(mIt != d.end()){
            auto mv = ModuleVariable{string("new "+thing), string(thing)};
            vector<ModulePortValue> mpv;
            auto gfxMv = GfxModule(mv, mousePosition.x, mousePosition.y, doc, mpv);
            gfxGraph.modules.push_back(gfxMv);
        }
    }

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
    virtual void mouseDrag(const MouseEvent & event) override;
    virtual void mouseUp(const MouseEvent & event) override;
    virtual void mouseMove(const MouseEvent & event) override;
    void paint (Graphics& g);
    void mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & d) override;

private:
    float scale;
    XY mousePosition;
    void updateBounds(const pair<XY, XY>& rectange);
    void updateBounds(const XY & position, const XY & size);

    void setGraph(const ConnectionGraphDescriptor& graph);
    void propagateUserModelChange();
    atomic_flag connectionGraphDescriptorLock = ATOMIC_FLAG_INIT;
    vector<ModulePortValue> valuesCopy; // TODO, figure out a better strategy like a map on GfxGraph

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
