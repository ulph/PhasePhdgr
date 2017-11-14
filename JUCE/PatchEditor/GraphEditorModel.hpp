#pragma once

#include <string>
#include <vector>
#include <math.h>
#include <float.h>
#include <list>
#include <random>
#include <assert.h>
#include <set>

#include <phasephckr.hpp>
#include "JuceHeader.h"

using namespace PhasePhckr;
using namespace std;

const float c_GridSize = 200;
const float c_NodeSize = 125;
const float c_PortSize = 10;

struct XY {
    XY() : x(0), y(0) {}
    XY(float x, float y) : x(x), y(y) {}
    float x;
    float y;
    XY& operator+=(const XY& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    XY& operator-=(const XY& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    operator Rectangle<float>() const {
        return Rectangle<float>(0, 0, x, y);
    }
};

inline XY operator+(XY lhs, const XY& rhs)
{
    lhs += rhs;
    return lhs;
}

inline XY operator-(XY lhs, const XY& rhs)
{
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

    bool within(XY p) const;
    void clearValue();
    void setValue(float v);
    float getValue();
    void draw(Graphics & g, int rowIndex=-1) const;
    void updateValue(const string& module, const std::vector<ModulePortValue> &mpvs);

    GfxPort();
    GfxPort(string port, const string unit, float value, bool isInput);

};


struct GfxModule {
    bool isParameter = false;

    vector<GfxPort> inputs;
    vector<GfxPort> outputs;

    ModuleVariable module;
    XY position;
    XY size = { c_NodeSize , c_NodeSize };

    const string& type() {
        return module.type;
    }

    void designPorts(
        const Doc &doc,
        const std::vector<ModulePortValue> &mpvs
    );

    void repositionPorts();
    bool within(XY p) const;
    bool withinPort(XY p, XY& portPosition, string &port, bool & inputPort);
    virtual void draw(Graphics & g, bool selected=false) const;
    bool getValue(const string& port, float& value);
    void clearValue(const string& port);
    bool setValue(const string& port, float value);

    GfxModule(
        const ModuleVariable & mv,
        float x, 
        float y, 
        const Doc & doc, 
        const std::vector<ModulePortValue> &mpvs
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

    bool within(XY p, bool & nearSource) const;
    void draw(Graphics & g) const;
    void calculatePath(const vector<GfxModule> & modules);
    GfxWire(const ModulePortConnection &connection, const vector<GfxModule> & modules);
};


struct GfxLooseWire {
    ModulePort attachedPort;
    bool attachedAtSource;
    XY position;
    XY destination;
    bool isValid = false;
    void draw(Graphics & g) const;
};


struct GfxGraph {
    vector<GfxModule> modules;
    list<GfxWire> wires;
    map<string, ComponentDescriptor> components;

    pair<XY, XY> getBounds();
    void moveDelta(XY delta);
    void moveIntoView();
    void recalculateWires(const vector<GfxModule>& modules);
    bool disconnect(const XY& mousePos, GfxLooseWire &looseWire);
    bool add(const string &type, const Doc & doc, const XY &pos);
    bool add(const ModuleVariable& module, const Doc & doc, const XY &pos, const std::vector<ModulePortValue> &mpv, bool absolutPositions=true);
    bool connect(const ModulePortConnection &connection);
    bool connect(const ModulePort &source, const ModulePort &target);
    bool connect(const GfxLooseWire &looseWire, const XY &mousePos);
    bool rename(string module, string newName);
    bool renameComponent(string componentType, string newComponentType);
    bool renameComponentPort(string componentType, string port, string newPort, bool inputPort);
    bool remove(const string &module);
    bool getModule(string name, const GfxModule** module);
    bool hasModuleName(string name);
    void designPorts(const Doc &doc);
    void createComponentFromSelection(const set<string> & selectedModules, Doc & doc, XY& position);
    PatchDescriptor exportModelData();
};
