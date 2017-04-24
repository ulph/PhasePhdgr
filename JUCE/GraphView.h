#ifndef GRAPHVIEW_H_INCLUDED
#define GRAPHVIEW_H_INCLUDED

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

struct XY {
    XY() : x(0), y(0){}
    XY(float x, float y) : x(x), y(y){}
    float x;
    float y;
};

class GraphView : public Component
{

public:
    GraphView(
        Viewport& parent,
        const PhasePhckr::Doc& doc,
        SubValue<PhasePhckr::ConnectionGraphDescriptor> * subscribedCGD,
        std::pair<std::string, std::string> inBus,
        std::pair<std::string, std::string> outBus
    )
        : gridSize(200.0f)
        , nodeSize(100.0f)
        , clickedComponent(nullptr)
        , doc(doc)
        , r(7.0f)
        , subscribedCGD(subscribedCGD)
        , inBus(inBus)
        , outBus(outBus)
        , parent(parent)
    {
        parent.setScrollOnDragEnabled(true);
        subscribedCGDhandle = subscribedCGD->subscribe(
            [this](const PhasePhckr::ConnectionGraphDescriptor& g){setGraph(g);}
        );
    }
    ~GraphView() {
        subscribedCGD->unsubscribe(subscribedCGDhandle);
    }
    virtual void mouseDown(const MouseEvent & event) override;
    virtual void mouseDrag(const MouseEvent & event) override;
    virtual void mouseUp(const MouseEvent & event) override;
    void paint (Graphics& g);
    void resized();

private:
    void setGraph(const PhasePhckr::ConnectionGraphDescriptor& graph);
    int subscribedCGDhandle;
    SubValue<PhasePhckr::ConnectionGraphDescriptor> * subscribedCGD;
    const std::string *clickedComponent;
    void recalculate();
    void recalculateBounds(bool force=false);
    Viewport& parent;
    XY upperBound;
    XY lowerBound;
    float gridSize;
    float nodeSize;
    float r;
    std::pair<std::string, std::string> inBus;
    std::pair<std::string, std::string> outBus;
    PhasePhckr::ConnectionGraphDescriptor graphDescriptor;
    PhasePhckr::Doc doc;
    std::map<std::string, XY> modulePosition;
    std::map< std::string, std::map<std::string, XY>> inputPortPositions;
    std::map< std::string, std::map<std::string, XY>> outputPortPositions;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphView)
};

#endif  // GRAPHVIEW_H_INCLUDED
