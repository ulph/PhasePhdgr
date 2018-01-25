#include "GraphEditorModel.hpp"
#include <math.h> 
#include <map>
#include <set>

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


bool GfxPort::within(XY p) const {
    return distance(p) <= c_PortSize;
}

float GfxPort::distance(XY p) const {
    float dx = p.x - (position.x + c_PortSize*0.5f);
    float dy = p.y - (position.y + c_PortSize*0.5f);
    return sqrt(dx*dx + dy*dy);
}

void GfxPort::clearValue(){
    value = defaultValue;
    assignedValue = false;
}

void GfxPort::setValue(float v){
    if(v!=value){
        assignedValue = true;
    }
    value = v;
}

float GfxPort::getValue(){
    if(assignedValue){
        return value;
    }
    return defaultValue;
}

void GfxPort::draw(Graphics & g, int rowIndex) {
    g.setColour(Colours::black);
    if (latched_mouseHover) {
        g.setColour(Colours::cyan);
    }
    else if(assignedValue){
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
        textX -= (int)(3*c_PortSize);
        textW *= 3;
        textY += rowIndex*(int)((isInput ? 1.75 : -1.75)*c_PortSize);
    }

    g.setColour(Colours::darkgrey);
    g.drawFittedText(
        port,
        textX,
        textY,
        textW,
        textH,
        Justification::centred,
        1
    );

    latched_mouseHover = false;
}

GfxPort::GfxPort()
    : port("?"), unit("?"), defaultValue(42), value(13), isInput(false)
{}

GfxPort::GfxPort(string port, const string unit, float value, bool isInput)
    : port(port), unit(unit), defaultValue(value), value(value), isInput(isInput)
{}

void GfxPort::updateValue(const string& module, const map<ModulePort, float> &mpvs){
    if(!isInput) return;
    for(const auto &kv:mpvs ){
        const ModulePortValue mpv(kv.first, kv.second);
        if(mpv.target.module == module && mpv.target.port == port){
            value = mpv.value;
            assignedValue = true;
        }
    }
}

void GfxModule::repositionPorts() {
    size_t max_p = (inputs.size() > outputs.size()) ? inputs.size() : outputs.size();
    if (max_p > 2.0f) {
        size.x = c_NodeSize*max_p*0.5f;
    }

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

XY GfxModule::midTop() const {
    return XY(position.x + 0.5f * size.x, position.y - 0.5f*c_PortSize);
}

XY GfxModule::midBottom() const {
    return XY(position.x + 0.5f * size.x, position.y + size.y - 0.5f*c_PortSize);
}

bool GfxModule::within(XY p) const {
    return (p.x > position.x
        && p.x < position.x + size.x
        && p.y > position.y
        && p.y < position.y + size.y
    );
}

bool GfxModule::withinPort(XY p, XY& portPosition, string &port, bool & inputPort) {
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

void GfxModule::draw(Graphics & g, bool selected) {
    if (latched_mouseHover) {
        auto c = Colours::cyan;
        g.setColour(c.withAlpha(0.25f));
    }
    else if (selected) {
        g.setColour(Colour(0x66888888));
    }
    else {
        g.setColour(Colour(0xAA000000));
    }
    float cornerSize = 5.f;
    if(isParameter) cornerSize = c_NodeSize*0.5f;

    g.fillRoundedRectangle(
        position.x,
        position.y,
        size.x,
        size.y,
        cornerSize
    );

    g.setColour(Colours::white); // restore trans
    Path path;
    path.addRoundedRectangle(
        position.x,
        position.y,
        size.x,
        size.y,
        cornerSize
    );

    PathStrokeType strokeType(1);
    strokeType.createStrokedPath(path, path);

    auto startColour = Colours::white;
    auto stopColour = Colours::grey;

    ColourGradient grad(
        startColour,
        position.x,
        position.y,
        stopColour,
        position.x,
        position.y + size.y,
        false
    );
    g.setGradientFill(grad);
    g.fillPath(path);

    g.setColour(Colours::white);
    if (state == CONFLICTINGCOMPONENT) g.setColour(Colours::red);
    else if (state == LOCALCOMPONENT) g.setColour(Colours::yellow);
    else if (state == UNKONWN) g.setColour(Colours::cyan);

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
    if (!isParameter) {
        for (auto &i : inputs) {
            i.draw(g, (row++ % numRows));
        }
    }

    row = 0;
    numRows = (outputs.size() > 4) ? 2 : 1;
    for (auto &o : outputs) {
        o.draw(g, (row++ % numRows));
    }

    latched_mouseHover = false;
}

bool GfxModule::getValue(const string& port, float& value){
    for(auto& ip:inputs){
        if(ip.port == port){
            value = ip.getValue();
            return true;
        }
    }
    return false;
}

void GfxModule::designPorts(const Doc &doc, const map<ModulePort, float> &mpvs){
    inputs.clear();
    outputs.clear();
    const auto d = doc.get();
    if (d.count(module.type)) {
        const auto & dd = d.at(module.type);
        for (auto p : dd.inputs) {
            inputs.emplace_back(GfxPort(p.name, p.unit, p.defaultValue, true)); // set default values
        }
        for (auto p : dd.outputs) {
            outputs.emplace_back(GfxPort(p.name, p.unit, p.defaultValue, false));
        }
    }
    for(auto &ip : inputs){
        ip.updateValue(module.name, mpvs);
    }
    repositionPorts();
}

GfxModule::GfxModule(
    const ModuleVariable & mv,
    float x,
    float y,
    const Doc & doc,
    const map<ModulePort, float> &mpvs
)
    : module(mv)
{
    if(module.type.front() == parameterMarker) isParameter = true;
    size = XY(c_NodeSize, c_NodeSize);
    position.y = y * c_PPGridSize;
    position.x = x * c_PPGridSize;
    designPorts(doc, mpvs);
}

float GfxWire::distance(XY p, XY& closestPoint) const {
    Point<float> p_ = Point<float>(p.x, p.y);
    Point<float> pp;
    path.getNearestPoint(p_, pp);
    closestPoint.x = pp.x;
    closestPoint.y = pp.y;
    float distanceFromClick = pp.getDistanceFrom(p_);
    return distanceFromClick;
}

bool GfxWire::within(XY p, bool & nearSource) const {
    XY pp;   
    float distanceFromClick = distance(p, pp);
    Point<float> pp_(pp.x, pp.y);
    float distanceFromPosition = pp_.getDistanceFrom(Point<float>(position.x, position.y));
    float distanceFromDestination = pp_.getDistanceFrom(Point<float>(destination.x, destination.y));
    if ( distanceFromClick < c_PortSize
    && ( (distanceFromPosition < 3*c_PortSize) || (distanceFromDestination < 3*c_PortSize)))
    {
        nearSource = distanceFromPosition < distanceFromDestination;
        return true;
    }
    return false;
}

void GfxWire::draw(Graphics & g) {
    if (latched_mouseHover) {
        g.setColour(Colours::cyan);
        g.fillPath(path);
    }
    else {
        g.setGradientFill(grad);
        g.fillPath(path);
    }
    latched_mouseHover = false;
}

void GfxWire::calculatePath(const vector<GfxModule> & modules) {
    bool foundSource = false;
    bool foundTarget = false;

    for (auto & m : modules) {
        if (!foundSource && m.module.name == connection.source.module) {
            for (const auto & p : m.outputs) {
                if (p.port == connection.source.port) {
                    position = p.position;
                    foundSource = true;
                    break;
                }
            }
            if (m.state == GfxModule::UNKONWN) {
                position = m.midBottom();
                foundSource = true;
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
            if (m.state == GfxModule::UNKONWN) {
                destination = m.midTop();
                foundTarget = true;
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
            Colour(0xBBFFFF00),
            position.x,
            position.y,
            Colour(0xBBFF0000),
            destination.x,
            destination.y,
            true
        );
    }
}

GfxWire::GfxWire(const ModulePortConnection &connection, const vector<GfxModule> & modules)
    : connection(connection)
{
    calculatePath(modules);
}

void GfxLooseWire::draw(Graphics & g) const {
    g.setColour(Colours::green);
    g.drawLine(
        position.x + c_PortSize*0.5f,
        position.y + c_PortSize*0.5f,
        destination.x + c_PortSize*0.5f,
        destination.y + c_PortSize*0.5f,
        c_PortSize * 0.5f
    );
}
