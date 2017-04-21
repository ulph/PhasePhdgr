#ifndef GRAPHVIEW_H_INCLUDED
#define GRAPHVIEW_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "design.hpp"
#include <iostream>
#include <set>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <climits>

struct XY {
    XY() : x(0), y(0){}
    XY(float x, float y) : x(x), y(y){}
    float x;
    float y;
};

class GraphView : public Component
{

public:
    GraphView()
        : gridSize(200)
        , nodeSize(100)
        , clickedComponent(nullptr)
    {}
    ~GraphView() {}
    virtual void mouseDown(const MouseEvent & event) override;
    virtual void mouseDrag(const MouseEvent & event) override;
    virtual void mouseUp(const MouseEvent & event) override;
    void paint (Graphics& g);
    void resized();
    void setGraph(const PhasePhckr::ConnectionGraphDescriptor& graph);

private:
    const std::string *clickedComponent;
    void recalculate();
    void recalculateBounds();
    XY upperBound;
    XY lowerBound;
    float gridSize;
    float nodeSize;
    PhasePhckr::ConnectionGraphDescriptor graphDescriptor;
    std::map<std::string, XY> modulePosition;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphView)
};

#endif  // GRAPHVIEW_H_INCLUDED
