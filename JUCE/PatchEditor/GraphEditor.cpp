#include "GraphEditor.hpp"
#include <algorithm>
#include <utility>

#include "PatchEditor.hpp"

using namespace PhasePhckr;


GraphEditorBundle::GraphEditorBundle(
    PatchEditor& graphEditor,
    SubValue<Doc> &subDoc,
    SubValue<PatchDescriptor> & subPatch,
    const vector<PadDescription> &inBus,
    const vector<PadDescription> &outBus
)
    : graphView(
        graphEditor
        , viewPort
        , subDoc
        , subPatch
        , inBus
        , outBus
    )
{
    addAndMakeVisible(viewPort);
    viewPort.setViewedComponent(&graphView, false);
    resized();
}


void GraphEditorBundle::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
}


void GraphEditorBundle::resized()
{
    viewPort.setBoundsRelative(0, 0, 1.0f, 1.0f);
    repaint();
}


bool makeModulePoopUp(PopupMenu & poop, const string & moduleName, const string & moduleType, PatchDescriptor& patch) {
    Label nameLbl(moduleName, moduleName);
    nameLbl.setEditable(true, true, false);

    Label typeLbl(moduleType, moduleType);
    typeLbl.setEditable(true, true, false);

    int ctr = 1;

    const int nameMenuId = ctr++;
    poop.addCustomItem(nameMenuId, &nameLbl, 20, 20, false);

    int typeMenuId = 999;
    int createInputMenuId = 999;
    int createOutputMenuId = 999;

    if (moduleType.front() == componentMarker) {
        typeMenuId = ctr++;
        poop.addCustomItem(typeMenuId, &typeLbl, 20, 20, false);

        createInputMenuId = ctr++;
        poop.addItem(createInputMenuId, "create input");

        createOutputMenuId = ctr++;
        poop.addItem(createOutputMenuId, "create output");
    }
    else if (moduleType.front() == parameterMarker) {
        // TODO, value, min, max editable
    }

    const int delMenuId = ctr++;
    poop.addItem(delMenuId, "remove module");

    int choice = poop.show();
    if (choice == delMenuId) {
        return 0 == patch.root.graph.remove(moduleName);
    }
    else if (choice == createInputMenuId || choice == createOutputMenuId) {
        if (!patch.components.count(moduleType)) return false;
        auto & comp = patch.components.at(moduleType);
        return 0 == patch.components.at(moduleType).addPort("newPort", choice == createInputMenuId, "", 0.0f);
    }

    if (moduleName != nameLbl.getText().toStdString()) {
        auto newModuleName = nameLbl.getText().toStdString();
        if (0 == patch.root.graph.rename(moduleName, newModuleName)) {
            if (patch.root.layout.count(moduleName)) {
                auto v = patch.root.layout.at(moduleName);
                patch.root.layout.erase(moduleName);
                patch.root.layout.insert_or_assign(newModuleName, v);
            }
            return true;
        }
        else return false;
    }
    else if (moduleType != typeLbl.getText().toStdString()) {
        return 0 == patch.renameComponentType(moduleType, typeLbl.getText().toStdString());
    }

    return false;
}


bool makeModuleSelectionPoopUp(PopupMenu &poop, set<const GfxModule *> &selection, PatchDescriptor &patch, Doc &doc, XY &position) {
    poop.addItem(1, "make component");
    poop.addItem(2, "delete");

    int choice = poop.show();
    switch (choice) {
    case 1:
    {
        set<string> selectedModules;
        for (const auto s : selection) {
            selectedModules.insert(s->module.name);
        }
        selection.clear();
        string newType = "";
        return 0 == patch.createNewComponentType(selectedModules, newType);
    }
    return true;
    case 2:
        for (const auto* m : selection) {
            patch.root.graph.remove(m->module.name);
        }
        return true;
    default:
        break;
    }
    return false;
}


bool makePortPoopUp(PopupMenu & poop, GfxModule & gfxModule, const string & port, PatchDescriptor& patch, bool inputPort) {
    const ModulePort mp(gfxModule.module.name, port);

    float value = 0.f;
    if (inputPort && !gfxModule.getValue(port, value)) return false; // error
    
    Label lbl(port + "_v", to_string(value));
    Label nameLbl(port, port);

    if (inputPort) {
        lbl.setEditable(true, true, false);
        poop.addItem(1, port);
        poop.addCustomItem(2, &lbl, 20, 20, false);
        poop.addItem(3, "clear value");
    }

    poop.addItem(4, "disconnect all");

    nameLbl.setEditable(true, true, false);
    if (gfxModule.module.type.front() == componentMarker) {
        poop.addCustomItem(5, &nameLbl, 20, 20, false);
        poop.addItem(6, "remove port");
    }

    int choice = poop.show();

    if (inputPort) {
        if (choice == 3) {
            return 0 == patch.root.graph.clearValue(mp);
        }

        if (value != lbl.getText().getFloatValue()) {
            return 0 == patch.root.graph.setValue(mp, lbl.getText().getFloatValue());
        }
    }

    if (choice == 4) {
        return 0 == patch.root.graph.disconnect(ModulePort(gfxModule.module.name, port), inputPort);
    }

    if (port != nameLbl.getText()) {
        if (!patch.components.count(gfxModule.module.type)) return false;
        return 0 == patch.renameComponentTypePort(gfxModule.module.type, port, nameLbl.getText().toStdString(), inputPort);
    }

    if (choice == 6) {
        if (!patch.components.count(gfxModule.module.type)) return false;
        auto& comp = patch.components.at(gfxModule.module.type);
        return 0 == comp.removePort(port, inputPort);
    }

    return false;
}


GraphEditor::GraphEditor(
    PatchEditor &patchEditor,
    Viewport &viewPort,
    SubValue<Doc> &subDoc,
    SubValue<PatchDescriptor> &subPatch,
    const vector<PadDescription> &inBus,
    const vector<PadDescription> &outBus
)
    : subPatch(subPatch)
    , subDoc(subDoc)
    , patchEditor(patchEditor)
    , viewPort(viewPort)
    , inBus(inBus)
    , outBus(outBus)
    , selecting(false)
    , patchIsDirty(false)
    , docIsDirty(false)
{
    subPatchHandle = subPatch.subscribe(
        [this](const PhasePhckr::PatchDescriptor& g){
            setGraph(g);
        }
    );

    docHandle = subDoc.subscribe(
        [this](const Doc& doc_){
            setDoc(doc_);
        }
    );

    clearZoom();

    setBounds(0, 0, 10, 10); // hack or we never get started
}


void GraphEditor::clearZoom() {
    zoom = defaultZoom;
    applyZoom();
}


void GraphEditor::increaseZoom() {
    zoom *= zoomIncrement;
    applyZoom();
}


void GraphEditor::decreaseZoom() {
    if (zoom < 0.1f) return;
    zoom /= zoomIncrement;
    applyZoom();
}


void GraphEditor::applyZoom() {
    setTransform(
        AffineTransform(
            zoom, 0, 0,
            0, zoom, 0
        )
    );
}


GraphEditor::~GraphEditor() {
    subPatch.unsubscribe(subPatchHandle);
    subDoc.unsubscribe(docHandle);
}


void GraphEditor::propagateUserModelChange() {
    for (const auto gm : modules) {
        patch.root.layout[gm.module.name] = 
            ModulePosition(
                gm.position.x, 
                gm.position.y
            );
    }
    patch.root.pruneLayout();
    subPatch.set(-1, patch);
    repaint();
}


void GraphEditor::mouseDoubleClick(const MouseEvent & event) {
    XY mousePos((float)event.x, (float)event.y);
    for (auto & m : modules) {
        if (m.within(mousePos)) {
            patchEditor.push_tab(m.module.name, m.module.type);
        }
    }
}


void GraphEditor::mouseDown(const MouseEvent & event) {
    viewPort.setScrollOnDragEnabled(true);
    if (event.mods.isMiddleButtonDown()) return;

    bool modelChanged = false;
    bool userInteraction = false;
    mouseDownPos = XY((float)event.x, (float)event.y);

    for (auto & m : modules) {
        string port;
        bool inputPort;
        XY position;
        // drag wire between ports
        if (m.withinPort(mouseDownPos, position, port, inputPort)) {
            if(event.mods.isRightButtonDown()){
                PopupMenu poop;
                modelChanged = makePortPoopUp(poop, m, port, patch, inputPort);
            }
            else{
                auto l = gfxGraphLock.make_scoped_lock();
                looseWire.isValid = true;
                looseWire.destination = mouseDownPos;
                looseWire.attachedAtSource = !inputPort;
                looseWire.attachedPort = { m.module.name, port };
                looseWire.position = position;
            }
            userInteraction = true;
            break;
        }
        // interract with a module
        else if (m.within(mouseDownPos)) {
            if(event.mods.isRightButtonDown()){
                PopupMenu poop;
                if (selectedModules.count(&m)) {
                    modelChanged = makeModuleSelectionPoopUp(poop, selectedModules, patch, doc, mouseDownPos);
                }
                else {
                    modelChanged = makeModulePoopUp(poop, m.module.name, m.module.type, patch);
                }
            }
            else if (event.mods.isShiftDown()) {
                auto l = gfxGraphLock.make_scoped_lock();
                if (selectedModules.count(&m)) {
                    selectedModules.erase(&m);
                }
                else {
                    selectedModules.insert(&m);
                }
            }
            else{
                draggedModule = &m;
            }
            userInteraction = true;
            break;
        }
    }

    // disconnect a wire
    if (!userInteraction) {
        auto scoped_lock = gfxGraphLock.make_scoped_lock();
        if (disconnect(mouseDownPos, looseWire)) {
            modelChanged = true;
            userInteraction = true;
        }
    }

    // select region start/stop
    if (!userInteraction) {
        if (event.mods.isShiftDown()) {
            selecting = true;
            selectionStart = event.position;
            selectionStop = event.position;
            repaint();
            userInteraction = true;
        }
        else {
            auto l = gfxGraphLock.make_scoped_lock();
            selectedModules.clear();
        }
    }

    // disable drag to scroll (until button released) in case of drag action
    if (userInteraction) {
        viewPort.setScrollOnDragEnabled(false);
        repaint();
    }

    if (modelChanged) propagateUserModelChange();
}


void GraphEditor::mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & d){
    if (d.deltaY >= 0) {
        increaseZoom();
    }
    else if (d.deltaY <= 0) {
        decreaseZoom();
    }
}


void GraphEditor::mouseDrag(const MouseEvent & event) {
    bool modelChanged = false;
    auto mousePos = XY((float)event.x, (float)event.y);
    if (draggedModule) {
        XY delta = mousePos - mouseDownPos;
        mouseDownPos = mousePos;
        draggedModule->position += delta;
        draggedModule->repositionPorts();
        auto mv = vector<GfxModule>{ *draggedModule };

        gfxGraphLock.lock();
        patch.root.layout.insert_or_assign(
            draggedModule->module.name,
            ModulePosition(draggedModule->position.x, draggedModule->position.y)
        );
        recalculateWires(mv);
        gfxGraphLock.unlock();

        updateBounds(getVirtualBounds());
        repaint();
    }
    if (looseWire.isValid) {
        auto l = gfxGraphLock.make_scoped_lock();
        looseWire.destination = mousePos;
        updateBounds(getVirtualBounds());
        findHoverDoodat(mousePos);
        repaint();
    }
    if (selecting) {
        selectionStop = event.position;
        repaint();
    }
    if (modelChanged) propagateUserModelChange();
}


void GraphEditor::mouseUp(const MouseEvent & event) {
    XY mousePos((float)event.x, (float)event.y);
    bool modelChanged = false;
    draggedModule = nullptr;
    if (looseWire.isValid) {
        auto l = gfxGraphLock.make_scoped_lock();
        modelChanged = connect(looseWire, mousePos);
    }
    if (selecting) {
        auto l = gfxGraphLock.make_scoped_lock();
        auto selectionRegion = Rectangle<float>(selectionStart, selectionStop);
        for (const auto &m : modules) {
            if(m.module.name == c_inBus.name || m.module.name == c_outBus.name) continue;
            Rectangle<float> mr(m.position.x, m.position.y, m.size.x, m.size.y);
            if (selectionRegion.intersectRectangle(mr)){
                selectedModules.insert(&m);
            }
        }
    }
    selecting = false;
    looseWire.isValid = false;
    moveIntoView(); // don't do this continously or stuff gets weird
    repaint();
    if (modelChanged) propagateUserModelChange();
}

void GraphEditor::findHoverDoodat(const XY& mousePos) {
    for (auto& m : modules) {
        for (auto& p : m.inputs) {
            if (p.within(mousePos)) {
                p.latched_mouseHover = true;
                repaint();
                mouseIsHovering = true;
                return;
            }
        }
        for (auto& p : m.outputs) {
            if (p.within(mousePos)) {
                p.latched_mouseHover = true;
                repaint();
                mouseIsHovering = true;
                return;
            }
        }
        if (m.within(mousePos)) {
            m.latched_mouseHover = true;
            repaint();
            mouseIsHovering = true;
            return;
        }
    }

    for (auto wit = wires.begin(); wit != wires.end(); ++wit) {
        bool nearestSource = false;
        if (wit->within(mousePos, nearestSource)) {
            wit->latched_mouseHover = true;
            repaint();
            mouseIsHovering = true;
            return;
        }
    }

    if (mouseIsHovering) {
        repaint();
    }

    mouseIsHovering = false;
}

void GraphEditor::mouseMove(const MouseEvent & event) {
    auto l = gfxGraphLock.make_scoped_lock();
    XY mousePos((float)event.x, (float)event.y);
    findHoverDoodat(mousePos);
}


void GraphEditor::updateBounds(const pair<XY, XY>& rectangle) {
    updateBounds(rectangle.first, rectangle.second);
}


void GraphEditor::itemDropped(const SourceDetails & dragSourceDetails){

    auto thing = dragSourceDetails.description.toString().toStdString();
    auto dropPos = dragSourceDetails.localPosition;

    {
        auto l = gfxGraphLock.make_scoped_lock();
        string name = "new_" + thing.substr(1) + "_";
        int i = 0;

        if (!moduleTypeIsValid(thing)) return;

        string fullName = name + to_string(i);

        while (patch.root.graph.add(fullName, thing)) {
            if (!moduleNameIsValid(fullName)) return;
            i++;
            fullName = name + to_string(i);
        }

        patch.root.layout[fullName] = ModulePosition(
            (float)dropPos.x - 0.5f*c_NodeSize,
            (float)dropPos.y - 0.5f*c_NodeSize
        );
    }

    propagateUserModelChange();
}


void GraphEditor::updateBounds(const XY & position, const XY & size){
    auto bounds = getBounds();
    if (bounds.getWidth() < (position.x + size.x)) {
        bounds.setWidth((int)(position.x + size.x) + 10);
    }
    if (bounds.getHeight() < (position.y + size.y)) {
        bounds.setHeight((int)(position.y + size.y) + 10);
    }
    setBounds(bounds);
}


void GraphEditor::paint(Graphics& g){
    auto scoped_lock = gfxGraphLock.make_scoped_lock();

    updateRenderComponents();

    g.fillAll(Colours::black);

    for(auto &w : wires){
        w.draw(g);
    }
    for (auto &mb : modules) {
        mb.draw(g, selectedModules.count(&mb));
    }
    if (looseWire.isValid) {
        looseWire.draw(g);
    }
    if (selecting) {
        g.setColour(Colour(0x44ffffff));
        g.fillRect(Rectangle<float>(selectionStart, selectionStop));
    }
}


void GraphEditor::setDoc(const Doc& newDoc){
    auto scoped_lock = gfxGraphLock.make_scoped_lock();

    doc = newDoc;
    auto inBus_ = Doc::makeBusModuleDoc(inBus, true);
    auto outBus_ = Doc::makeBusModuleDoc(outBus, false);
    doc.add(inBus_);
    doc.add(outBus_);

    docIsDirty = true;

    repaint();
}


void GraphEditor::setGraph(const PatchDescriptor& newPatch) {
  auto scoped_lock = gfxGraphLock.make_scoped_lock();

  patch = newPatch;
  patchIsDirty = true;

  repaint();
}


void GraphEditor::updateRenderComponents() 
{
    // call from within paint

    if (docIsDirty) {
        designPorts(doc);
        recalculateWires(modules);
        docIsDirty = false;
    }

    if (patchIsDirty) {

        patch.root.graph.modules[c_inBus.name] = c_inBus.type;
        patch.root.graph.modules[c_outBus.name] = c_outBus.type;

        const string start = c_inBus.name;
        const string stop = c_outBus.name;
        ModulePositionMap mp;
        setNodePositions(patch.root.graph, mp, start, stop);

        modules.clear();
        wires.clear();

        for (const auto & kv : patch.root.graph.modules) {
            ModuleVariable m(kv);
            XY xy(mp.at(m.name).x, mp.at(m.name).y);
            if (patch.root.layout.count(m.name)) {
                auto xy_ = patch.root.layout.at(m.name);
                xy.x = (float)xy_.x / (float)c_GridSize;
                xy.y = (float)xy_.y / (float)c_GridSize;
            }
            modules.push_back(GfxModule(m, xy.x, xy.y, doc, patch.root.graph.values));
        }

        auto bounds = getVirtualBounds();
        XY delta(0, 0);
        if (bounds.first.x < 0) delta.x = -bounds.first.x;
        if (bounds.first.y < 0) delta.y = -bounds.first.y;
        if (delta.x && delta.y) {
            moveDelta(delta);
            bounds = getVirtualBounds();
        }

        for (const auto &c : patch.root.graph.connections) {
            wires.emplace_back(GfxWire(c, modules));
        }

        updateBounds(bounds);
        patchIsDirty = false;
    }

}

// migrated from GfxGraph

pair<XY, XY> GraphEditor::getVirtualBounds() {
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

void GraphEditor::moveDelta(XY delta) {
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

void GraphEditor::moveIntoView() {
    XY delta = { 0, 0 };
    auto b = getVirtualBounds();
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

void GraphEditor::recalculateWires(const vector<GfxModule>& modules) {
    for (auto &w : wires) {
        w.calculatePath(modules);
    }
}

bool GraphEditor::disconnect(const XY& mousePos, GfxLooseWire &looseWire) {
    for (auto wit = wires.begin(); wit != wires.end(); ++wit) {
        bool nearestSource = false;
        // disconnect a wire
        if (wit->within(mousePos, nearestSource)) {
            auto ret = patch.root.graph.disconnect(wit->connection);
            if (ret != 0) return false;
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
            return true;
        }
    }
    return false;
}

bool GraphEditor::connect(const GfxLooseWire &looseWire, const XY &mousePos) {
    for (const auto& m : modules) {
        if (looseWire.attachedAtSource) {
            for (const auto& ip : m.inputs) {
                if (ip.within(mousePos)) {
                    return 0 == patch.root.graph.connect(
                        ModulePortConnection(
                            looseWire.attachedPort, 
                            ModulePort(m.module.name, ip.port)
                        )
                    );
                }
            }
        }
        else {
            for (const auto& op : m.outputs) {
                if (op.within(mousePos)) {
                    return 0 == patch.root.graph.connect(
                        ModulePortConnection(
                            ModulePort(m.module.name, op.port),
                            looseWire.attachedPort
                        )
                    );
                }
            }
        }
    }
    return false;
}

void GraphEditor::designPorts(const Doc &doc) {
    for (auto& m : modules) {
        map<ModulePort, float> mpvs;
        for (const auto& ip : m.inputs) {
            if (ip.assignedValue) {
                mpvs[ModulePort(m.module.name, ip.port)] = ip.value;
            }
        }
        m.designPorts(doc, mpvs);
    }
}

// end