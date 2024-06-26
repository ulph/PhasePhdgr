#pragma once

#include <string>
#include <vector>
#include <math.h>
#include <float.h>
#include <list>
#include <random>
#include <assert.h>
#include <set>

#include <phasephdgr.hpp>

#include <juce_graphics/juce_graphics.h>

using namespace juce;
using namespace PhasePhdgr;
using namespace std;

const float c_PPGridSize = 200;
const float c_NodeSize = 125;
const float c_PortSize = 10;

struct XY {
    XY() : x(0), y(0) {}
    XY(float x, float y) : x(x), y(y) {}
    
    float x;
    float y;
    
    XY& operator+=(const XY& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    
    XY& operator-=(const XY& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    
    operator Rectangle<float>() const {
        return Rectangle<float>(0, 0, x, y);
    }
};

inline XY operator+(XY lhs, const XY& rhs) {
    lhs += rhs;
    return lhs;
}

inline XY operator-(XY lhs, const XY& rhs) {
    lhs -= rhs;
    return lhs;
}


struct GfxPort {
    string port;
    string unit;
    float defaultValue;
    float value;
    bool isInput;
    bool assignedValue = false;
    XY position;

    int edge = 0;
    int row = -1;

    bool latched_mouseHover = false;

    bool within(XY p) const;
    float distance(XY p) const;
    void clearValue();
    void setValue(float v);
    float getValue();
    void draw(Graphics & g);
    void updateValue(const string& module, const map<ModulePort, float> &mpvs);

    GfxPort();
    GfxPort(string port, const string unit, float value, bool isInput);
};


struct GfxModule {
    bool isParameter = false;
    bool latched_mouseHover = false;

    enum moduleState {
        MODULE,
        LOCALCOMPONENT,
        GLOBALCOMPONENT,
        CONFLICTINGCOMPONENT,
        UNKONWN
    };

    moduleState state = MODULE;

    vector<GfxPort> inputs;
    vector<GfxPort> outputs;

    ModuleVariable module;
    XY position;
    XY size = { c_NodeSize , c_NodeSize };

    XY midTop() const;
    XY midBottom() const;

    void designPorts(
        const Doc &doc,
        const map<ModulePort, float> &mpvs
    );

    void repositionPorts();
    bool within(XY p) const;
    bool withinPort(XY p, XY& portPosition, string &port, bool & inputPort);
    virtual void draw(Graphics & g, bool selected=false);
    bool getValue(const string& port, float& value);

    GfxModule(
        const ModuleVariable & mv,
        float x, 
        float y, 
        const Doc & doc, 
        const map<ModulePort, float> &mpvs
    );

};


struct GfxWire {
private:
    Path path;
    ColourGradient grad;
public:
    ModulePortConnection connection;
    XY position;
    XY destination;

    bool latched_mouseHover = false;

    float distance(XY p, XY& closestPoint) const;
    bool within(XY p, bool & nearSource) const;
    void draw(Graphics & g);
    void calculatePath(const vector<GfxModule> & modules, unsigned int index);
    GfxWire(const ModulePortConnection &connection, const vector<GfxModule> & modules, unsigned int index);
};

struct GfxLooseWire {
    ModulePort attachedPort;
    bool attachedAtSource;
    XY position;
    XY destination;
    bool isValid = false;
    void draw(Graphics & g) const;
};