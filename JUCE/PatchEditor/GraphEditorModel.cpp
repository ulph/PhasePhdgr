#include "GraphEditorModel.hpp"


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
    return (
       p.x > (position.x - 0.5f*c_PortSize)
    && p.x < (position.x + c_PortSize*1.5f)
    && p.y > (position.y - 0.5f*c_PortSize)
    && p.y < (position.y + c_PortSize*1.5f)
    );
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

void GfxPort::draw(Graphics & g, int rowIndex) const {
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
}

GfxPort::GfxPort()
    : port("?"), unit("?"), defaultValue(42), value(13), isInput(false)
{}

GfxPort::GfxPort(string port, const string unit, float value, bool isInput)
    : port(port), unit(unit), defaultValue(value), value(value), isInput(isInput)
{}

void GfxPort::updateValue(const string& module, const std::vector<ModulePortValue> &mpvs){
    if(!isInput) return;
    for(const auto &mpv:mpvs ){
        if(mpv.target.module == module && mpv.target.port == port){
            value = mpv.value;
            assignedValue = true;
        }
    }
}


void GfxModule::repositionPorts() {
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

void GfxModule::draw(Graphics & g, bool selected) const {
    if (selected) {
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

bool GfxModule::getValue(const string& port, float& value){
    for(auto& ip:inputs){
        if(ip.port == port){
            value = ip.getValue();
            return true;
        }
    }
    return false;
}

void GfxModule::clearValue(const string& port){
    for(auto& ip:inputs){
        if(ip.port == port){
            ip.clearValue();
            return;
        }
    }
}

bool GfxModule::setValue(const string& port, float value){
    for(auto& ip:inputs){
        if(ip.port == port){
            ip.setValue(value);
            return true;
        }
    }
    return false;
}


void GfxModule::designPorts(const Doc &doc, const std::vector<ModulePortValue> &mpvs){
    inputs.clear();
    outputs.clear();
    const auto d = doc.get();
    if (d.count(module.type)) {
        const auto & dd = d.at(module.type);
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
        size.x = c_NodeSize + c_NodeSize*max_p/3.0f;
    }
    repositionPorts();
}


GfxModule::GfxModule(
    const ModuleVariable & mv,
    float x,
    float y,
    const Doc & doc,
    const std::vector<ModulePortValue> &mpvs
)
    : module(mv)
{
    if(module.type.at(0) == '=') isParameter = true;
    size = XY(c_NodeSize, c_NodeSize);
    position.y = y * c_GridSize;
    position.x = x * c_GridSize;
    designPorts(doc, mpvs);
}


bool GfxWire::within(XY p, bool & nearSource) const {
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
void GfxWire::draw(Graphics & g) const {
    g.setGradientFill(grad);
    g.fillPath(path);
}
void GfxWire::calculatePath(const vector<GfxModule> & modules) {
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


pair<XY, XY> GfxGraph::getBounds() {
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

void GfxGraph::moveDelta(XY delta) {
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

void GfxGraph::moveIntoView() {
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

void GfxGraph::recalculateWires(const vector<GfxModule>& modules) {
    for (auto &w : wires) {
        w.calculatePath(modules);
    }
}

bool GfxGraph::disconnect(const XY& mousePos, GfxLooseWire &looseWire) {
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

bool GfxGraph::add(const string &type, const Doc & doc, const XY &pos) {
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

bool GfxGraph::add(const ModuleVariable& module, const Doc & doc, const XY &pos, const std::vector<ModulePortValue> &mpv, bool absolutPositions) {
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

bool GfxGraph::connect(const ModulePortConnection &connection) {
    wires.emplace_back(GfxWire(connection, modules));
    return true;
}

bool GfxGraph::connect(const ModulePort &source, const ModulePort &target) {
    ModulePortConnection newCon;
    newCon.source = source;
    newCon.target = target;
    return connect(newCon);
}

bool GfxGraph::connect(const GfxLooseWire &looseWire, const XY &mousePos) {
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

bool GfxGraph::rename(string module, string newName){
    bool foundModule = false;

    int mIdx = 0;
    int i=0;

    for (const auto& m : modules) {
        if (m.module.name == module) {
            // found it
            foundModule = true;
            mIdx = i;
        }
        if (m.module.name == newName) {
            // conflict
            return false;
        }
        i++;
    }

    if(!foundModule) return false;

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

    return true;
}

bool GfxGraph::renameComponent(string componentType, string newComponentType){
    if(componentType[0] != '@') return false;
    if(newComponentType[0] != '@') return false;
    if(components.count(componentType) && !components.count(newComponentType)){
        for(auto& m: modules){
            if(m.module.type == componentType){
                m.module.type = newComponentType;
            }
        }
        components[newComponentType] = components[componentType];
        components.erase(componentType);
        return true;
    }
    return false;
}

bool GfxGraph::remove(const string &module) {
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

bool GfxGraph::getModule(string name, const GfxModule** module) {
    for (const auto &m : modules) {
        if (m.module.name == name) {
            *module = &m;
            return true;
        }
    }
    return false;
}

bool GfxGraph::hasModuleName(string name) {
    for (const auto &m : modules) {
        if (m.module.name == name) {
            return true;
        }
    }
    return false;
}

void GfxGraph::designPorts(const Doc &doc){
    for(auto& m : modules){
        std::vector<ModulePortValue> mpvs;
        for(const auto& ip: m.inputs){
            if(ip.assignedValue){
                mpvs.push_back(ModulePortValue{{m.module.name, ip.port}, ip.value});
            }
        }
        m.designPorts(doc, mpvs);
    }
}

void GfxGraph::createComponentFromSelection(const set<string> & selectedModules, Doc & doc, XY& position){
    ConnectionGraphDescriptor cgd;

    // copy over modules and any non-default values
    for (const auto name : selectedModules) {
        const GfxModule * m;
        if(getModule(name, &m)){
            cgd.modules.push_back(m->module);
            for (const auto &ip : m->inputs) {
                if (ip.assignedValue) {
                    cgd.values.push_back(ModulePortValue{{m->module.name, ip.port}, ip.value });
                }
            }
        }
    }

    set<string> inAlias;
    set<string> outAlias; // TODO, make this a map, so _same_ module/port can be merged...
    vector<PadDescription> inBus;
    vector<PadDescription> outBus;

    string name = "newComponent";
    int ctr = 0;
    while (hasModuleName(name + to_string(ctr))){
        ctr++;
    }
    name += to_string(ctr);

    // TODO - expose any unconnected ports with set values ... chances are quite good that a user wants to at least set them per instance of component
    for (auto &w : wires) {
        // copy any internal connections
        if (selectedModules.count(w.connection.source.module) && selectedModules.count(w.connection.target.module)) {
            cgd.connections.push_back(w.connection);
        }
        // store the external connections
        else if (selectedModules.count(w.connection.target.module)) {
            auto mp = w.connection.target;
            string alias = mp.port;
            while (inAlias.count(alias)) { alias += "_"; }
            inAlias.insert(alias);
            PadDescription pd = { alias, "", 0};
            inBus.push_back(pd);
            // store 'api' connection
            cgd.connections.push_back(ModulePortConnection{{c_inBus.name, alias}, {w.connection.target.module, w.connection.target.port}});
            // remap external connection graph
            w.connection.target.module = name;
            w.connection.target.port = alias;
        }
        else if (selectedModules.count(w.connection.source.module)) {
            auto mp = w.connection.source;
            string alias = mp.port;
            while (outAlias.count(alias)) { alias += "_"; }
            outAlias.insert(alias);
            PadDescription pd = { alias, "", 0};
            outBus.push_back(pd);
            // store 'api' connectino
            cgd.connections.push_back(ModulePortConnection{{w.connection.source.module, w.connection.source.port}, {c_outBus.name, alias}});
            // remap external connection graph
            w.connection.source.module = name;
            w.connection.source.port = alias;
        }
    }

    // create a ComponentDescriptor
    string type = "@INCOMPONENT";
    ctr = 0;
    while (components.count(type + to_string(ctr))) {
        ctr++;
    }
    type += to_string(ctr);

    ComponentDescriptor cmp;
    cmp.graph = cgd;
    cmp.docString = "";
    cmp.inBus = inBus;
    cmp.outBus = outBus;

    // store it on model
    components[type] = cmp;

    // add it to graph
    vector<ModulePortValue> mpv;
    ModuleVariable mv{ name, type };
    GfxModule gfxM(
       mv,
       (position.x - 0.5f*c_NodeSize) / c_GridSize,
       (position.y - 0.5f*c_NodeSize) / c_GridSize,
       doc,
       mpv
    );

    modules.push_back(gfxM);

    for (const auto &m : selectedModules) {
        remove(m);
    }

}

PatchDescriptor GfxGraph::exportModelData(){
    PatchDescriptor graph;
    for (const auto &m :modules) {
        graph.layout.emplace(m.module.name, ModulePosition{ (int)m.position.x, (int)m.position.y });
        if (m.module.name == "inBus" || m.module.name == "outBus") continue;
        graph.root.graph.modules.emplace_back(m.module);
        for ( const auto &ip : m.inputs){
            if(ip.assignedValue){
                graph.root.graph.values.emplace_back(ModulePortValue{{m.module.name, ip.port}, ip.value});
            }
        }
    }
    for (const auto &w : wires) {
        graph.root.graph.connections.emplace_back(w.connection);
    }

    graph.components = components;

    return graph;
}


