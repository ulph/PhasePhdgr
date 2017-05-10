#pragma once

#include <string>
#include <vector>
#include "JuceLibraryCode/JuceHeader.h"
#include "design.hpp"
#include <math.h>
#include <float.h>
#include <list>
#include <random>
#include <assert.h>

using namespace PhasePhckr;
using namespace std;

const float c_GridSize = 200;
const float c_NodeSize = 125;
const float c_PortSize = 10;

static void calcCable(Path & path, float x0, float y0, float x1, float y1, float r, float nodeSize) {
    x0 += 0.5f*r;
    y0 += 0.5f*r;
    x1 += 0.5f*r;
    y1 += 0.5f*r;

    PathStrokeType strokeType(1);
    strokeType.setStrokeThickness(1.5f);

    path.startNewSubPath(x0, y0);

    if (y1 <= y0) {
        float dy = 0.25f*nodeSize;
        float dx = 0.25f*nodeSize * (x1 >= x0 ? 1.f : -1.f);
        float s = x1 != x0 ? 1.f : -1.f;
        float x[7]{
            x0 + dx,
            x0 + 2.f * dx,

            x0 + 3.f * dx,
            x1 - 3.f * dx*s,

            x1 - 2.f * dx*s,
            x1 - dx*s,
            x1
        };
        float y[7]{
            y0 + 2.f * dy,
            y0 + dy,

            y0,
            y1,

            y1 - dy,
            y1 - 2 * dy,
            y1
        };
        path.quadraticTo(x[0], y[0], x[1], y[1]);
        path.cubicTo(x[2], y[2], x[3], y[3], x[4], y[4]);
        path.quadraticTo(x[5], y[5], x[6], y[6]);
    }
    else {
        float d = 0.5f*nodeSize;
        path.cubicTo(x0, y0 + d, x1, y1 - d, x1, y1);
    }
    strokeType.createStrokedPath(path, path);
}


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

    bool within(XY p) const {
        return (
           p.x > (position.x - 0.5f*c_PortSize)
        && p.x < (position.x + c_PortSize*1.5f)
        && p.y > (position.y - 0.5f*c_PortSize)
        && p.y < (position.y + c_PortSize*1.5f)
        );
    }

    void clearValue(){
        value = defaultValue;
        assignedValue = false;
    }

    void setValue(float v){
        if(v!=value){
            assignedValue = true;
        }
        value = v;
    }

    float getValue(){
        if(assignedValue){
            return value;
        }
        return defaultValue;
    }

    void draw(Graphics & g, int rowIndex=-1) const {
        g.setColour(Colours::black);
        if(assignedValue){
            g.setColour(Colours::darkgrey);
        }
        g.fillEllipse(position.x, position.y, c_PortSize, c_PortSize);
        g.setColour(Colours::grey);
        g.drawEllipse(position.x, position.y, c_PortSize, c_PortSize, 1.0f);

        int textX = (int)(position.x - c_PortSize);
        int textY = (int)(position.y + (isInput ? 1.f : -1.f)*1.5f*c_PortSize + (isInput ? 0 : -0.5*c_PortSize));
        int textW = (int)(3.f * c_PortSize);
        int textH = (int)c_PortSize;
        if (rowIndex >= 0) {
            textX -= (int)(1.5*c_PortSize);
            textW *= 2;
            textY += rowIndex*(int)((isInput ? 1.75 : -1.75)*c_PortSize);
        }

        g.drawFittedText(
            port, 
            textX,
            textY,
            textW,
            textH,
            Justification::centred,
            1
        );
    }

    GfxPort()
        : port("?"), unit("?"), value(13), defaultValue(42), isInput(false)
    {}

    GfxPort(string port, const string unit, float value, bool isInput) 
        : port(port), unit(unit), value(value), defaultValue(value), isInput(isInput)
    {}

    void updateValue(const string& module, const std::vector<ModulePortValue> &mpvs){
        if(!isInput) return;
        for(const auto &mpv:mpvs ){
            if(mpv.target.module == module && mpv.target.port == port){
                value = mpv.value;
                assignedValue = true;
            }
        }
    }
};


struct GfxModule {

    vector<GfxPort> inputs;
    vector<GfxPort> outputs;

    ModuleVariable module;
    XY position;
    XY size = { c_NodeSize , c_NodeSize };

    void repositionPorts() {
        for (auto it = inputs.begin(); it != inputs.end(); ++it)
        {
            float n = (float)distance(inputs.begin(), it);
            it->position.x = position.x + (n + 0.5f) / inputs.size() * size.x - 0.5f*c_PortSize;
            it->position.y = position.y - 0.5f*c_PortSize;
        }

        for (auto it = outputs.begin(); it != outputs.end(); ++it)
        {
            float n = (float)distance(outputs.begin(), it);
            it->position.x = position.x + (n + 0.5f) / outputs.size() * size.x - 0.5f*c_PortSize;
            it->position.y = position.y + size.y - 0.5f*c_PortSize;
        }
    }

    bool within(XY p) const {
        return (p.x > position.x
            && p.x < position.x + size.x
            && p.y > position.y
            && p.y < position.y + size.y
        );
    }

    bool withinPort(XY p, XY& portPosition, string &port, bool & inputPort) {
        for (auto & ip : inputs) {
            if (ip.within(p)) {
                portPosition = ip.position;
                port = ip.port;
                inputPort = true;
                return true;
            }
        }
        for (auto & op : outputs) {
            if (op.within(p)) {
                portPosition = op.position;
                port = op.port;
                inputPort = false;
                return true;
            }
        }
        return false;
    }

    void draw(Graphics & g, bool selected=false) const {
        if (selected) {
            g.setColour(Colour(0x88888888));
        }
        else {
            g.setColour(Colour(0x88000000));
        }
        g.fillRoundedRectangle(
            position.x, 
            position.y,
            size.x,
            size.y, 
            5.f
        );

        g.setColour(Colours::white); // restore trans
        Path path;
        path.addRoundedRectangle(
            position.x,
            position.y,
            size.x,
            size.y,
            2.f
        );

        PathStrokeType strokeType(1);
        strokeType.createStrokedPath(path, path);

        ColourGradient grad(
            Colours::white, 
            position.x, 
            position.y, 
            Colours::grey, 
            position.x, 
            position.y + size.y, 
            false
        );
        g.setGradientFill(grad);
        g.fillPath(path);
        
        g.setColour(Colours::white);
        g.drawFittedText(
            module.name + "\n" + module.type, 
            (int)position.x, 
            (int)position.y, 
            (int)size.x, 
            (int)size.y, 
            Justification::centred, 
            1
        );

        int row = 0;
        int numRows = (inputs.size() > 4) ? 2 : 1;
        for (const auto &i : inputs) {
            i.draw(g, (row++ % numRows));
        }

        row = 0;
        numRows = (outputs.size() > 4) ? 2 : 1;
        for (const auto &o : outputs) {
            o.draw(g, (row++ % numRows));
        }

    }

    bool getValue(const string& port, float& value){
        for(auto& ip:inputs){
            if(ip.port == port){
                value = ip.getValue();
                return true;
            }
        }
        return false;
    }

    void clearValue(const string& port){
        for(auto& ip:inputs){
            if(ip.port == port){
                ip.clearValue();
                return;
            }
        }
    }

    bool setValue(const string& port, float value){
        for(auto& ip:inputs){
            if(ip.port == port){
                ip.setValue(value);
                return true;
            }
        }
        return false;
    }

    GfxModule(
        const ModuleVariable & mv, 
        float x, 
        float y, 
        const Doc & doc, 
        const std::vector<ModulePortValue> &mpvs
    )
        : module(mv)
    {
        size = XY(c_NodeSize, c_NodeSize);
        position.y = y * c_GridSize;
        position.x = x * c_GridSize;
        const auto d = doc.get();
        if (d.count(mv.type)) {
            const auto & dd = d.at(mv.type);
            for (auto p : dd.inputs) {
                inputs.emplace_back(GfxPort(p.name, p.unit, p.value, true)); // set default values
            }
            for (auto p : dd.outputs) {
                outputs.emplace_back(GfxPort(p.name, p.unit, p.value, false));
            }
        }
        for(auto &ip : inputs){
            ip.updateValue(module.name, mpvs);
        }
        size_t max_p = (inputs.size() > outputs.size()) ? inputs.size() : outputs.size();
        if (max_p > 3.0f) {
            size.x = c_NodeSize + c_NodeSize*(max_p - 3.0f) / 5.0f;
        }
        repositionPorts();
    }

};


struct GfxWire {
private:
    Path path;
    ColourGradient grad;
public:
    ModulePortConnection connection;
    XY position;
    XY destination;
    bool within(XY p, bool & nearSource) const {
        Point<float> p_ = Point<float>(p.x, p.y);
        Point<float> pp;
        path.getNearestPoint(p_, pp);
        float distanceFromClick = pp.getDistanceFrom(p_);
        float distanceFromPosition = pp.getDistanceFrom(Point<float>(position.x, position.y));
        float distanceFromDestination = pp.getDistanceFrom(Point<float>(destination.x, destination.y));
        if ( distanceFromClick < c_PortSize
        && ( (distanceFromPosition < 3*c_PortSize) || (distanceFromDestination < 3*c_PortSize)))
        {
            nearSource = distanceFromPosition < distanceFromDestination;
            return true;
        }
        return false;
    }
    void draw(Graphics & g) const {
        g.setGradientFill(grad);
        g.fillPath(path);
    }
    void calculatePath(const vector<GfxModule> & modules) {
        bool foundSource = false;
        bool foundTarget = false;
        for (const auto & m : modules) {
            if (!foundSource && m.module.name == connection.source.module) {
                for (const auto & p : m.outputs) {
                    if (p.port == connection.source.port) {
                        position = p.position;
                        foundSource = true;
                        break;
                    }
                }
            }
            if (!foundTarget && m.module.name == connection.target.module) {
                for (const auto & p : m.inputs) {
                    if (p.port == connection.target.port) {
                        destination = p.position;
                        foundTarget = true;
                        break;
                    }
                }
            }
            if (foundTarget && foundSource) break;
        }

        if (foundSource || foundTarget) {
            path.clear();
            calcCable(
                path, 
                position.x, 
                position.y, 
                destination.x, 
                destination.y, 
                c_PortSize, 
                c_NodeSize
            );
            grad = ColourGradient(
                Colours::yellow, 
                position.x, 
                position.y,
                Colours::red, 
                destination.x, 
                destination.y,
                true
            );
        }
    }

    GfxWire(const ModulePortConnection &connection, const vector<GfxModule> & modules)
        : connection(connection) 
    {
        calculatePath(modules);
    }

};


struct GfxLooseWire {
    ModulePort attachedPort;
    bool attachedAtSource;
    XY position;
    XY destination;
    bool isValid = false;
    void draw(Graphics & g) const {
        g.setColour(Colours::green);
        g.drawLine(
            position.x + c_PortSize*0.5f,
            position.y + c_PortSize*0.5f,
            destination.x + c_PortSize*0.5f,
            destination.y + c_PortSize*0.5f,
            c_PortSize * 0.5f
        );
    }
};


struct GfxGraph {
    vector<GfxModule> modules;
    list<GfxWire> wires;
    map<string, ComponentDescriptor> components;

    pair<XY, XY> getBounds() {
        XY min(FLT_MAX, FLT_MAX);
        XY max(FLT_MIN, FLT_MIN);
        for (auto &mb : modules) {
            if ((mb.position.x + mb.size.x) > max.x) {
                max.x = (mb.position.x + mb.size.x);
            }
            if ((mb.position.y + mb.size.y) > max.y) {
                max.y = (mb.position.y + mb.size.y);
            }
            if (mb.position.x < min.x) {
                min.x = mb.position.x;
            }
            if (mb.position.y < min.y) {
                min.y = mb.position.y;
            }
        }
        return make_pair(min, max);
    }

    void moveDelta(XY delta) {
        for (auto &mb : modules) {
            mb.position += delta;
            mb.repositionPorts();
        }
        for (auto &w : wires) {
            w.position += delta;
            w.destination += delta;
            w.calculatePath(modules);
        }
    }

    void moveIntoView() {
        XY delta = {0, 0};
        auto b = getBounds();
        if (b.first.x < 0) {
            delta.x = -b.first.x;
        }
        if (b.first.y < 0) {
            delta.y = -b.first.y;
        }
        if (delta.x || delta.y) {
            moveDelta(delta);
        }
    }
    
    void recalculateWires(const vector<GfxModule>& modules) {
        for (auto &w : wires) {
            w.calculatePath(modules);
        }
    }
    
    bool disconnect(const XY& mousePos, GfxLooseWire &looseWire) {
        for (auto wit = wires.begin(); wit != wires.end(); ++wit) {
            bool nearestSource = false;
            // disconnect a wire
            if (wit->within(mousePos, nearestSource)) {
                looseWire.isValid = true;
                looseWire.destination = mousePos;
                looseWire.attachedAtSource = !nearestSource;
                if (looseWire.attachedAtSource) {
                    looseWire.attachedPort = wit->connection.source;
                    looseWire.position = wit->position;
                }
                else {
                    looseWire.attachedPort = wit->connection.target;
                    looseWire.position = wit->destination;
                }
                wires.erase(wit);
                return true;
            }
        }
        return false;
    }

    bool add(const string &type, const Doc & doc, const XY &pos) {
        bool didAdd = false;
        auto d = doc.get();
        auto mIt = d.find(type);
        if (mIt != d.end()) {
            vector<ModulePortValue> mpv;
            string name = "new" + type;
            int ctr = 0;
            while (hasModuleName(name + to_string(ctr))) {
                ctr++;
            }
            name += to_string(ctr);
            didAdd = add(
                ModuleVariable{ name, type },
                doc,
                pos,
                mpv
            );
        }
        return didAdd;
    }

    bool add(const ModuleVariable& module, const Doc & doc, const XY &pos, const std::vector<ModulePortValue> &mpv, bool absolutPositions=true) {
        if (hasModuleName(module.name)) {
            assert(0);
            return false;
        }
        auto position = pos;
        if (absolutPositions) {
            position.x = (position.x - 0.5f*c_NodeSize) / c_GridSize;
            position.y = (position.y - 0.5f*c_NodeSize) / c_GridSize;
        }
        auto gfxMv = GfxModule(
            module,
            position.x,
            position.y,
            doc,
            mpv
        );
        modules.emplace_back(gfxMv);
        return true;
    }

    bool connect(const ModulePortConnection &connection) {
        wires.emplace_back(GfxWire(connection, modules));
        return true;
    }

    bool connect(const ModulePort &source, const ModulePort &target) {
        ModulePortConnection newCon;
        newCon.source = source;
        newCon.target = target;
        return connect(newCon);
    }

    bool connect(const GfxLooseWire &looseWire, const XY &mousePos) {
        bool foundPort = false;
        for (auto &m : modules) {
            if (foundPort) break;
            if (looseWire.attachedAtSource) {
                for (const auto& p : m.inputs) {
                    if (p.within(mousePos)) {
                        connect(looseWire.attachedPort, ModulePort{ m.module.name, p.port });
                        foundPort = true;
                        break;
                    }
                }
            }
            else {
                for (const auto& p : m.outputs) {
                    if (p.within(mousePos)) {
                        connect(ModulePort{ m.module.name, p.port }, looseWire.attachedPort);
                        foundPort = true;
                        break;
                    }
                }
            }
        }
        return foundPort;
    }

    bool rename(string module, string newName){
        bool foundModule = false;
        bool conflict = false;
        int mIdx = 0;
        int i=0;
        for (const auto& m : modules) {
            if (m.module.name == module) {
                foundModule = true;
                mIdx = i;
            }
            if (m.module.name == newName) {
                conflict = true;
            }
            i++;
        }
        if (foundModule && !conflict) {
            modules.at(mIdx).module.name = newName;
            auto wit = wires.begin();
            while (wit != wires.end()) {
                if (wit->connection.source.module == module){
                    wit->connection.source.module = newName;
                }
                else if(wit->connection.target.module == module) {
                    wit->connection.target.module = newName;
                }
                wit++;
            }
        }
        return foundModule && !conflict;
    }

    bool remove(const string &module) {
        bool foundModule = false;
        int mIdx = 0;
        for (const auto& m : modules) {
            if (m.module.name == module) {
                foundModule = true;
                break;
            }
            else {
                mIdx++;
            }
        }
        if (foundModule) {
            auto wit = wires.begin();
            while (wit != wires.end()) {
                if (wit->connection.source.module == module || wit->connection.target.module == module) {
                    wit = wires.erase(wit);
                }
                else {
                    ++wit;
                }
            }
            modules.erase(modules.begin() + mIdx);
        }
        return foundModule;
    }

    bool getModule(string name, const GfxModule** module) {
        for (const auto &m : modules) {
            if (m.module.name == name) {
                *module = &m;
                return true;
            }
        }
        return false;
    }

    bool hasModuleName(string name) {
        for (const auto &m : modules) {
            if (m.module.name == name) {
                return true;
            }
        }
        return false;
    }

};
