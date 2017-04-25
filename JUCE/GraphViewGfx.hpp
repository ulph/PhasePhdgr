#pragma once

#include <string>
#include <vector>
#include "JuceLibraryCode/JuceHeader.h"
#include "design.hpp"
#include <math.h>


using namespace PhasePhckr;

const float c_GridSize = 200;
const float c_NodeSize = 100;
const float c_PortSize = 7;


static void calcCable(Path & path, float x0, float y0, float x1, float y1, float r, float nodeSize) {
    x0 += 0.5f*r;
    y0 += 0.5f*r;
    x1 += 0.5f*r;
    y1 += 0.5f*r;

    PathStrokeType strokeType(1);
    strokeType.setStrokeThickness(1);

    path.startNewSubPath(x0, y0);

    if (y1 <= y0) {
        float dy = 0.25*nodeSize;
        float dx = 0.25*nodeSize * (x1 >= x0 ? 1 : -1);
        float s = x1 != x0 ? 1 : -1;
        float x[7]{
            x0 + dx,
            x0 + 2 * dx,

            x0 + 3 * dx,
            x1 - 3 * dx*s,

            x1 - 2 * dx*s,
            x1 - dx*s,
            x1
        };
        float y[7]{
            y0 + 2 * dy,
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
        float d = 0.5*nodeSize;
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

class GfxLooseWire {
public:
    bool attachedAtSource;
    XY position;
    XY destination;
    void draw(Graphics & gl) const {
        /* */
    }
};


class GfxWire {
private:
    Path path;
    ModulePortConnection connection;
public:
    XY position;
    XY destination;
    bool within(XY p, bool & nearSource) const {
        return false;
    }
    void draw(Graphics & gl) const {
        /* */
    }
    GfxWire() {

    }
};


class GfxPort {
private:
    std::string port;
    bool connected;
    float value;
public:
    float size = c_PortSize;
    XY position;
    bool within(XY p) const {
        return (p.x > position.x
            && p.x < position.x + size
            && p.y > position.y
            && p.y < position.y + size
            );
    }
    void draw(Graphics & g) const {
        g.setColour(Colours::black);
        g.fillEllipse(position.x, position.y, size, size);
        g.setColour(Colours::grey);
        g.drawEllipse(position.x, position.y, size, size, 1.0f);
        g.drawText(port, position.x - size, position.y + 1.5f*size, 3 * size, size, Justification::centred);
    }
    XY getPosition() const { return position; }
    std::string getPort() const { return port; }
};


class GfxModule {

private:
    ModuleVariable module;
    std::vector<GfxPort> inputs;
    std::vector<GfxPort> outputs;

public:
    XY position;
    XY size = { c_NodeSize , c_NodeSize };

    void resizeAndRepositionPorts() {
        size_t max_p = (inputs.size() > outputs.size()) ? inputs.size() : outputs.size();
        if(max_p > 3.0f){
            size.x += c_NodeSize*(max_p - 3.0f) / 6.0f;
        }

        for (auto it = inputs.begin(); it != inputs.end(); ++it)
        {
            float n = std::distance(inputs.begin(), it);
            it->position.x = position.x + (n + 0.5f) / inputs.size() * size.x - 0.5f*it->size;
            it->position.y = position.y - 0.5*it->size;
        }

        for (auto it = outputs.begin(); it != outputs.end(); ++it)
        {
            float n = std::distance(outputs.begin(), it);
            it->position.x = position.x + (n + 0.5f) / outputs.size() * size.x - 0.5f*it->size;
            it->position.y = position.y + size.y - 0.5*it->size;
        }
    }

    bool within(XY p) const {
        return (p.x > position.x
            && p.x < position.x + size.x
            && p.y > position.y
            && p.y < position.y + size.y
            );
    }

    bool withinPort(XY p, XY& portPosition, std::string& portName, bool & inputPort) const {
        for (const auto & ip : inputs) {
            if (ip.within(p)) {
                portPosition = ip.getPosition();
                portName = ip.getPort();
                inputPort = true;
                return true;
            }
        }
        for (const auto & op : outputs) {
            if (op.within(p)) {
                portPosition = op.getPosition();
                portName = op.getPort();
                inputPort = false;
                return true;
            }
        }
        return false;
    }

    void draw(Graphics & g) const {
        g.setColour(Colour((uint8_t)0, (uint8_t)0, (uint8_t)0, (float)0.5f));
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
        ColourGradient grad(Colours::white, position.x, position.y, Colours::grey, position.x, position.y + size.y, false);
        g.setGradientFill(grad);
        g.fillPath(path);
        
        g.setColour(Colours::white);
        g.drawFittedText(module.name, position.x, position.y, size.x, size.y, Justification::centred, 1);
        for (const auto &i : inputs) {
            i.draw(g);
        }
        for (const auto &o : outputs) {
            o.draw(g);
        }
    }

    GfxModule(const ModuleVariable & mv, int x, int y, float h, float w, const Doc & doc, const std::vector<ModulePortValue> &mpv) {
        module = mv;
        size = XY(h*c_NodeSize, w*c_NodeSize);
        position.y = y * c_GridSize;
        position.x = x * c_GridSize;
        const auto d = doc.get();
        if (d.count(mv.type)) {
            const auto & dd = d.at(mv.type);
            for (auto p : dd.inputs) {
                inputs.emplace_back(GfxPort());
            }
            for (auto p : dd.outputs) {
                outputs.emplace_back(GfxPort());
            }
        }
        resizeAndRepositionPorts();
    }

};


struct GfxGraph {
    std::vector<GfxModule> modules;
    std::vector<GfxWire> wires;
};
