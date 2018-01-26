#include "GraphEditorModel.hpp"
#include <math.h> 
#include <map>
#include <set>

#include <random>

const float c_fontSize = 1.4f*c_PortSize;

static float calcCableOffset(unsigned int index) {
    auto sign = ((index % 2) == 0) ? -1.0f : 1.0f;
    auto normIndex = (float)((index + 1) >> 1);
    auto scale = 10.0f;
    return sign * normIndex * scale;
}

static void calcCable(Path & path, float x0, float y0, float x1, float y1, float r, float nodeSize, unsigned int index = 0) {
    float o = calcCableOffset(index);

    x0 += 0.5f*r;
    y0 += 0.5f*r;
    x1 += 0.5f*r;
    y1 += 0.5f*r;

    PathStrokeType strokeType(1);
    strokeType.setStrokeThickness(1.5f);

    path.startNewSubPath(x0, y0);

    float delta = sqrtf((x0 - x1)*(x0 - x1) + (y0 - y1)*(y0 - y1));
    float deltaY = fabsf(y0 - y1);
    float deltaYNorm = sqrtf(sqrtf(deltaY)); // (over)compensate for cubicness, which makes it behave a bit stretchy

    // eq for y1 < y0
    float dxA = 0.25f * nodeSize * (x1 >= x0 ? 1.0f : -1.0f);
    float dyA = (0.25f + 0.25f*deltaYNorm)*nodeSize;

    // eq for y1 >= y0
    float dxB = 0.0f;
    float dyB = 0.125f*deltaYNorm*nodeSize;

    float minDy = 0.75f*nodeSize;

    // fade between both equations
    float dx = 0.0f;
    float dy = 0.0f;
    float ymix = fminf(1.0f, fmaxf(0.0f, ((y0-y1)/(2*minDy) + 1.0f)));
    float xmix = fminf(1.0f, fmaxf(0.0f, ((y0-y1)/(2*minDy) + 0.0f)));
    dx = xmix*dxA + (1.0f - xmix)*dxB;
    dy = ymix*dyA + (1.0f - ymix)*dyB;

    // constrain dy so cable doesn't "collapse" when y0 and y1 are very close
    dy = dy < minDy ? minDy : dy;

    // when (x0, y0) and (x1, y1) are close: dy->0, dx->0, o->0
    float s = fminf(1.0f, delta / (1.25f*minDy));
    dy *= s*s;

    path.cubicTo(x0 + o + dx, y0 + dy, x1 + o - dx, y1 - dy, x1, y1);
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

void GfxPort::draw(Graphics & g) {
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

    int textW = (int)(0.475f * c_NodeSize);
    int textY = (int)(position.y + (isInput ? 1.f : -1.f)*1.5f*c_PortSize + (isInput ? 0 : -0.5*c_PortSize));
    int textH = (int)c_PortSize;
    if (row >= 0) {
        textW = (int)(0.9f*c_NodeSize);
        textY += row*(int)((isInput ? 1.75 : -1.75)*c_PortSize);
    }
    if(edge != 0) textW = (int)(0.475f * c_NodeSize);
    int textX = (int)(position.x - 0.5f*textW + 0.5f*c_PortSize);

    g.setColour(Colours::darkgrey);

    g.setFont(c_fontSize);
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
    if (max_p > 2.0f && !isParameter) {
        size.x = c_NodeSize*max_p*0.5f;
    }

    int ctr = 0;
    int numRows = (inputs.size() > 2) ? 2 : 1;
    for (auto it = inputs.begin(); it != inputs.end(); ++it)
    {
        float n = (float)distance(inputs.begin(), it);
        it->edge = 0;
        it->position.x = position.x + (n + 0.5f) / inputs.size() * size.x - 0.5f*c_PortSize;
        it->position.y = position.y - 0.5f*c_PortSize;
    }
    if (inputs.size() > 1) {
        inputs[0].edge = -1;
        inputs[inputs.size() - 1].edge = 1;
    }

    ctr = 0;
    numRows = (outputs.size() > 4) ? 2 : 1;
    for (auto it = outputs.begin(); it != outputs.end(); ++it)
    {
        float n = (float)distance(outputs.begin(), it);
        it->edge = 0;
        it->position.x = position.x + (n + 0.5f) / outputs.size() * size.x - 0.5f*c_PortSize;
        it->position.y = position.y + size.y - 0.5f*c_PortSize;
    }
    if (outputs.size() > 1) {
        outputs[0].edge = -1;
        outputs[outputs.size() - 1].edge = 1;
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
        g.setColour(Colour(0xCC002222));
    }
    else if (selected) {
        g.setColour(Colour(0xCC888888));
    }
    else {
        g.setColour(Colour(0xCC000000));
    }
    float cornerSize = 10.f;
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

    g.setFont(c_fontSize);
    g.drawFittedText(
        module.name + "\n" + module.type,
        (int)position.x,
        (int)position.y,
        (int)size.x,
        (int)size.y,
        Justification::centred,
        1
    );

    if (!isParameter) {
        for (auto &i : inputs) {
            i.draw(g);
        }
    }

    for (auto &o : outputs) {
        o.draw(g);
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

void GfxWire::calculatePath(const vector<GfxModule> & modules, unsigned int index) {
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
            c_NodeSize,
            index
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

GfxWire::GfxWire(const ModulePortConnection &connection, const vector<GfxModule> & modules, unsigned int index)
    : connection(connection)
{
    calculatePath(modules, index);
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
