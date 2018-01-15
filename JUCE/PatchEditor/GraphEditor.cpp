#include "GraphEditor.hpp"
#include <algorithm>
#include <utility>

#include <limits.h>

#include "PatchEditor.hpp"

using namespace PhasePhckr;


bool GraphEditor::makeModulePoopUp(PopupMenu & poop, const string & moduleName, const string & moduleType) {
    bool isBusModule = (moduleName == c_inBus.name || moduleName == c_outBus.name);
    if (rootComponentName == rootMarker && isBusModule) return false;
    if (!rootComponent()) return false;

    bool isComponentBus = rootComponentName != rootMarker && isBusModule;
    bool isComponent = moduleType.front() == componentMarker || isComponentBus;

    auto validModule = rootComponent()->graph.modules.count(moduleName);

    if (!isComponentBus) {
        if (!validModule) return false;
    }
    else {
        if(!validModule && !(isBusModule)) return false;
    }

    TextLabelMenuEntry nameLbl;
    nameLbl.title.setText("Name:", NotificationType::dontSendNotification);
    nameLbl.edit.setText(moduleName, NotificationType::dontSendNotification);
    nameLbl.edit.setEditable(true, true, false);

    int ctr = 1;

    const int nameMenuId = ctr++;
    const int delMenuId = ctr++;
    if (moduleName != c_inBus.name && moduleName != c_outBus.name) {
        poop.addCustomItem(nameMenuId, &nameLbl, 200, 20, false);
        poop.addItem(delMenuId, "remove module");
    }
    int cloneComponentMenuId = 999;
    int addComponentInputMenuId = 999;
    int addComponentOutputMenuId = 999;

    ComponentPopupMenuState cmpState;

    PopupMenu cmpPoop;

    if (isComponent && !isComponentBus) {
        cloneComponentMenuId = ctr++;
        cmpPoop.addItem(cloneComponentMenuId, c_componentMenuStrings.clone);

        makeComponentPopupMenu(cmpPoop, ctr, cmpState, moduleType, patch, globalComponents, patch.components);

    }
    else if (isComponentBus){
        if (moduleName == c_inBus.name) {
            addComponentInputMenuId = ctr++;
            cmpPoop.addItem(addComponentInputMenuId, c_componentMenuStrings.createInput);
        }
        else if (moduleName == c_outBus.name) {
            addComponentOutputMenuId = ctr++;
            cmpPoop.addItem(addComponentOutputMenuId, c_componentMenuStrings.createOutput);
        }
    }

    if(isComponent) poop.addSubMenu("Component", cmpPoop);

    int choice = poop.show();

    if (applyComponentPopuMenuChoice(choice, cmpState, moduleType, patch, globalComponents)) return true;

    if (choice == delMenuId) {
        return 0 == rootComponent()->graph.remove(moduleName);
    }
    else if (choice == cloneComponentMenuId) {
        ComponentDescriptor cd;
        if (patch.components.count(moduleType)) cd = patch.components.at(moduleType);
        else if (globalComponents.count(moduleType)) cd = globalComponents.at(moduleType);
        else return false;
        string newType = moduleType + "_Clone";
        if (0 != patch.addComponentType(newType, cd, true)) return false;
        if (!rootComponent()->graph.modules.count(moduleName)) return true;
        rootComponent()->graph.modules[moduleName] = newType;
        return true;
    }
    else if (choice == addComponentInputMenuId || choice == addComponentOutputMenuId) {
        auto * comp = rootComponent();
        if (comp == nullptr) return false;
        return 0 == comp->addPort("newPort", choice == addComponentInputMenuId, "", 0.0f);
    }

    auto newModuleName = nameLbl.edit.getText().toStdString();
    if (moduleName != newModuleName) {
        if (0 == rootComponent()->graph.rename(moduleName, newModuleName)) {
            if (rootComponent()->layout.count(moduleName)) {
                auto v = rootComponent()->layout.at(moduleName);
                rootComponent()->layout.erase(moduleName);
                rootComponent()->layout[newModuleName] = v;
            }
            return true;
        }
        else return false;
    }

    return false;
}

ComponentDescriptor* GraphEditor::rootComponent() {
    // lock before call
    if (rootComponentName == rootMarker) return &patch.root;
    if (patch.components.count(rootComponentName)) return &patch.components[rootComponentName];
    return nullptr;
}

GraphEditor::GraphEditor(
    PatchEditor &patchEditor,
    const string& rootComponent_,
    const PatchDescriptor& initialPatch,
    Viewport &viewPort,
    const Doc &initialDoc,
    SubValue<PatchDescriptor> &subPatch,
    const vector<PadDescription> &inBus,
    const vector<PadDescription> &outBus,
    const LayoutUpdateCallback &layoutUpdateCb
)
    : subPatch(subPatch)
    , doc(initialDoc)
    , patchEditor(patchEditor)
    , viewPort(viewPort)
    , inBus(inBus)
    , outBus(outBus)
    , selecting(false)
    , patchIsDirty(false)
    , docIsDirty(false)
    , rootComponentName(rootComponent_)
    , layoutUpdateCallback(layoutUpdateCb)
{
    setDoc(initialDoc);
    setGraph(initialPatch);

    subPatchHandle = subPatch.subscribe(
        [this](const PhasePhckr::PatchDescriptor& g){
            setGraph(g);
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
}

void GraphEditor::updateLayout() {
    auto l = gfxGraphLock.make_scoped_lock();
    for (const auto gm : modules) {
        rootComponent()->layout[gm.module.name] =
            ModulePosition(
                gm.position.x,
                gm.position.y
            );
    }
}

void GraphEditor::propagatePatch() {
    updateLayout();
    PatchDescriptor patchCopy;
    {
        auto l = gfxGraphLock.make_scoped_lock();
        patchCopy = patch;
    }
    subPatch.set(-1, patchCopy); // we want it back (lazily forces refresh/sync)
    repaint();
}

void GraphEditor::propagateLayout() {
    updateLayout();
    map<string, ModulePosition> layoutCopy;
    {
        auto l = gfxGraphLock.make_scoped_lock();
        layoutCopy = rootComponent()->layout;
    }
    layoutUpdateCallback(rootComponentName, layoutCopy);
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

    {
        mouseDownPos = XY((float)event.x, (float)event.y);
        auto l = gfxGraphLock.make_scoped_lock();
        GfxPort* pickedPort = nullptr;
        GfxModule* pickedModule = nullptr;
        GfxWire* pickedWire = nullptr;
        bool nearestSource = false;
        findCloseThings(mouseDownPos, &pickedPort, &pickedModule, &pickedWire, nearestSource);
        if (pickedPort && pickedModule) {
            if (event.mods.isRightButtonDown()) {

                // port popup menu
                if (portPopupMenuData.build(portPopupMenu, patch, rootComponentName, *pickedModule, pickedPort->port, pickedPort->isInput)) {
                    auto portPopupCallback = ModalCallbackFunction::forComponent<GraphEditor>(
                        [](int choice, GraphEditor* editor) {
                        bool modelChanged = false;
                            if (editor == nullptr) return;
                            {
                                auto l = editor->gfxGraphLock.make_scoped_lock();
                                modelChanged = editor->portPopupMenuData.handleChoice(editor->patch, editor->rootComponent(), choice);
                            }
                            if (modelChanged) {
                                editor->propagatePatch();
                            }
                        }
                        , this
                    );
                    portPopupMenu.showMenuAsync(PopupMenu::Options(), portPopupCallback);
                }

            }
            else {
                looseWire.isValid = true;
                looseWire.destination = mouseDownPos;
                looseWire.attachedAtSource = !pickedPort->isInput;
                looseWire.attachedPort = { pickedModule->module.name, pickedPort->port };
                looseWire.position = pickedPort->position;
            }
            userInteraction = true;
        }
        else if (pickedModule) {
            if (event.mods.isRightButtonDown()) {

                // selection popup menu
                if (selectedModules.count(pickedModule)) {
                    PopupMenu poop;
                    poop.addItem(1, "make component");
                    poop.addItem(2, "delete");
                    auto selectionPopupCallback = ModalCallbackFunction::forComponent<GraphEditor>(
                        [](int choice, GraphEditor* editor) {
                            bool modelChanged = false;
                            if (editor == nullptr) return;
                            {
                                auto l = editor->gfxGraphLock.make_scoped_lock();
                                switch (choice) {
                                case 1:
                                    {
                                        set<string> selectedModules;
                                        for (const auto s : editor->selectedModules) {
                                            selectedModules.insert(s->module.name);
                                        }
                                        editor->selectedModules.clear();
                                        string newType = "";
                                        modelChanged = 0 == editor->patch.createNewComponentType(editor->rootComponent(), selectedModules, newType);
                                    }
                                    break;
                                case 2:
                                    for (const auto* m : editor->selectedModules) {
                                        editor->rootComponent()->graph.remove(m->module.name);
                                    }
                                    modelChanged = true;
                                    break;
                                default:
                                    break;
                                }
                            }
                            if (modelChanged) {
                                editor->propagatePatch();
                            }
                        }
                        , this
                    );
                    poop.showMenuAsync(PopupMenu::Options(), selectionPopupCallback);
                }

                else {
//                    modelChanged = makeModulePoopUp(poop, pickedModule->module.name, pickedModule->module.type);
                }
            }
            else if (event.mods.isShiftDown()) {
                if (selectedModules.count(pickedModule)) {
                    selectedModules.erase(pickedModule);
                }
                else {
                    selectedModules.insert(pickedModule);
                }
            }
            else if (event.mods.isCtrlDown()) {
                looseWire.isValid = true;
                looseWire.destination = mouseDownPos;
                looseWire.attachedAtSource = true;
                looseWire.attachedPort = { pickedModule->module.name, "#@!?" }; // intentionally invalid port
                looseWire.position = mouseDownPos;
            }
            else {
                draggedModule = pickedModule;
            }
            userInteraction = true;
        }
        else if (pickedWire) {
            if (disconnect(pickedWire, mouseDownPos, nearestSource)) {
                modelChanged = true;
                userInteraction = true;
            }
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

    if (modelChanged) propagatePatch();
}

void GraphEditor::mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & d){
    if (fabs(d.deltaX) < 0.001 && fabs(d.deltaY) < 0.001) return;
    if(e.mods.isAltDown()){
        if (d.deltaY >= 0) {
            increaseZoom();
        }
        else if (d.deltaY <= 0) {
            decreaseZoom();
        }
    }
    else {
        // TODO scroll/pan instead
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
        auto l = gfxGraphLock.make_scoped_lock();
        rootComponent()->layout[draggedModule->module.name] = ModulePosition(draggedModule->position.x, draggedModule->position.y);
        recalculateWires(mv);
        updateBounds(getVirtualBounds());
    }
    if (looseWire.isValid) {
        auto l = gfxGraphLock.make_scoped_lock();
        looseWire.destination = mousePos;
        updateBounds(getVirtualBounds());
        findHoverDoodat(mousePos);
    }
    if (selecting) {
        selectionStop = event.position;
    }
    if (draggedModule || looseWire.isValid || selecting) {
        auto vpm = viewPort.getMouseXYRelative();
        auto vpb = viewPort.getWidth();

        int mX = vpm.x;
        int mY = vpm.y;

        int dx = 0;
        int dy = 0;

        int bW = viewPort.getWidth();
        int bH = viewPort.getHeight();

        int border = 25;

        int minX = border;
        int minY = border;
        int maxX = bW - border;
        int maxY = bH - border;

        if (minX > mX) dx = minX - mX;
        if (minY > mY) dy = minY - mY;
        if (maxX < mX) dx = maxX - mX;
        if (maxY < mY) dy = maxY - mY;

        if ( abs(dx) > 2 || abs(dy) > 2 ){
            setTopLeftPosition(getX() + (int)(dx*0.5f), getY() + (int)(dy*0.5f));
            beginDragAutoRepeat(10);
        }

        repaint();
    }
    if (modelChanged) propagatePatch();
}

void GraphEditor::mouseUp(const MouseEvent & event) {
    XY mousePos((float)event.x, (float)event.y);
    bool modelChanged = false;
    if (looseWire.isValid) {
        auto l = gfxGraphLock.make_scoped_lock();
        GfxPort* pickedPort = nullptr;
        GfxModule* pickedModule = nullptr;
        GfxWire* pickedWire = nullptr;
        bool nearestSource = false;
        findCloseThings(mousePos, &pickedPort, &pickedModule, &pickedWire, nearestSource);
        if (pickedModule && pickedPort) {
            modelChanged = connect(pickedModule, pickedPort);
        }
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
    if (modelChanged) propagatePatch();
    else if (draggedModule) propagateLayout();
    draggedModule = nullptr;
}

void GraphEditor::findCloseThings(const XY& pos, GfxPort** closestPort, GfxModule** closestModule, GfxWire** closestWire, bool& nearestSource) {
    // ports and modules
    for (auto& m : modules) {
        vector< vector<GfxPort> *> vs = { &m.inputs, &m.outputs };
        for (auto v : vs) {
            float distance = FLT_MAX;
            for (auto& p : *v) {
                auto d = p.distance(pos);
                if (p.within(pos) && d < distance) {
                    distance = d;
                    *closestPort = &p;
                    *closestModule = &m;
                }
            }
        }
        if ( !(*closestModule) && m.within(pos) ) *closestModule = &m;
    }
 
    if (*closestPort || *closestModule) return;

    // wires
    float distance = FLT_MAX;
    XY p;
    for (auto wit = wires.begin(); wit != wires.end(); ++wit) {
        auto d = wit->distance(pos, p);
        if (wit->within(pos, nearestSource) && d < distance) {
            distance = d;
            *closestWire = &(*wit);
        }
    }
}

void GraphEditor::findHoverDoodat(const XY& mousePos) {
    GfxPort* pickedPort = nullptr;
    GfxModule* pickedModule = nullptr;
    GfxWire* pickedWire = nullptr;
    bool nearestSource = false;

    findCloseThings(mousePos, &pickedPort, &pickedModule, &pickedWire, nearestSource);

    if (pickedPort) {
        pickedPort->latched_mouseHover = true;
        repaint();
        mouseIsHovering = true;
        return;
    }
    if (pickedModule) {
        pickedModule->latched_mouseHover = true;
        repaint();
        mouseIsHovering = true;
        return;
    }
    if (pickedWire) {
        pickedWire->latched_mouseHover = true;
        repaint();
        mouseIsHovering = true;
        return;
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

    auto type = dragSourceDetails.description.toString().toStdString();
    auto pos = dragSourceDetails.localPosition;

    {
        auto l = gfxGraphLock.make_scoped_lock();

        string name = "";
        if (0 != rootComponent()->addModule(type, name)) return;

        rootComponent()->layout[name] = ModulePosition(
            (float)pos.x - 0.5f*c_NodeSize,
            (float)pos.y - 0.5f*c_NodeSize
        );
    }

    propagatePatch();
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

void GraphEditor::setGlobalComponents(const map<string, ComponentDescriptor>& globalComponents_) {
    auto scoped_lock = gfxGraphLock.make_scoped_lock();
    globalComponents = globalComponents_;
    docIsDirty = true;
    repaint();
}

void GraphEditor::setDoc(const Doc& newDoc){
    auto scoped_lock = gfxGraphLock.make_scoped_lock();

    doc = newDoc;

    auto * rt = rootComponent();
    if (rootComponentName != rootMarker && rt != nullptr){
        inBus = rt->inBus;
        outBus = rt->outBus;
    }

    ModuleDoc inBus_;
    ModuleDoc outBus_;
    inBus_.fromBusModulePorts(inBus, true);
    outBus_.fromBusModulePorts(outBus, false);

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

        modules.clear();
        wires.clear();
        patchIsDirty = false;

        if (rootComponent() == nullptr) return;

        rootComponent()->graph.modules[c_inBus.name] = c_inBus.type;
        rootComponent()->graph.modules[c_outBus.name] = c_outBus.type;

        const string start = c_inBus.name;
        const string stop = c_outBus.name;
        ModulePositionMap mp;
        setNodePositions(rootComponent()->graph, mp, start, stop);

        for (const auto & kv : rootComponent()->graph.modules) {
            ModuleVariable m(kv);
            XY xy(mp.at(m.name).x, mp.at(m.name).y);
            if (rootComponent()->layout.count(m.name)) {
                auto xy_ = rootComponent()->layout.at(m.name);
                xy.x = (float)xy_.x / (float)c_PPGridSize;
                xy.y = (float)xy_.y / (float)c_PPGridSize;
            }
            auto gfxM = GfxModule(m, xy.x, xy.y, doc, rootComponent()->graph.values);
            if (globalComponents.count(m.type) && patch.components.count(m.type)) {
                gfxM.state = GfxModule::CONFLICTINGCOMPONENT;
            }
            else if (!globalComponents.count(m.type) && patch.components.count(m.type)) {
                gfxM.state = GfxModule::LOCALCOMPONENT;
            }
            else if (globalComponents.count(m.type)) {
                gfxM.state = GfxModule::GLOBALCOMPONENT;
            }
            else if (!doc.get().count(m.type)){
                gfxM.state = GfxModule::UNKONWN;
            }
            modules.push_back(gfxM);
        }

        auto bounds = getVirtualBounds();
        XY delta(0, 0);
        if (bounds.first.x < 0) delta.x = -bounds.first.x;
        if (bounds.first.y < 0) delta.y = -bounds.first.y;
        if (delta.x && delta.y) {
            moveDelta(delta);
            bounds = getVirtualBounds();
        }

        for (const auto &c : rootComponent()->graph.connections) {
            wires.emplace_back(GfxWire(c, modules));
        }

        updateBounds(bounds);
    }

}

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

void GraphEditor::recalculateWires(vector<GfxModule>& modules) {
    for (auto &w : wires) {
        w.calculatePath(modules);
    }
}

bool GraphEditor::disconnect(GfxWire* wire, const XY& mousePos, bool nearestSource) {
    auto ret = rootComponent()->graph.disconnect(wire->connection);
    if (ret != 0) return false;
    looseWire.isValid = true;
    looseWire.destination = mousePos;
    looseWire.attachedAtSource = !nearestSource;
    if (looseWire.attachedAtSource) {
        looseWire.attachedPort = wire->connection.source;
        looseWire.position = wire->position;
    }
    else {
        looseWire.attachedPort = wire->connection.target;
        looseWire.position = wire->destination;
    }
    return true;
}

bool GraphEditor::connect(GfxModule* module, GfxPort* port) {
    if ( !module || !port ) return false;

    if (looseWire.attachedAtSource && port->isInput) {
        return 0 == rootComponent()->graph.connect(
            ModulePortConnection(
                looseWire.attachedPort,
                ModulePort(module->module.name, port->port)
            )
        );
    }
    else if(!looseWire.attachedAtSource && !port->isInput) {
        return 0 == rootComponent()->graph.connect(
            ModulePortConnection(
                ModulePort(module->module.name, port->port),
                looseWire.attachedPort
            )
        );
    }
    return false;
}

bool GraphEditor::autoConnect(const string &source, const string &target) {
    if(!rootComponent()->graph.modules.count(source) 
    || !rootComponent()->graph.modules.count(target)
    ) return false;

    string sourceType = "";
    string targetType = "";

    for (const auto& m : modules) {
        if (sourceType.size() && targetType.size()) break;
        if (m.module.name == source) sourceType = m.module.type;
        if (m.module.name == target) targetType = m.module.type;
    }
    
    const auto& d = doc.get();

    if (!d.count(sourceType) || !d.count(targetType)) return false;

    if (d.at(targetType).inputs.size() == 1 && d.at(sourceType).outputs.size() == 1) {
        rootComponent()->graph.connect(
            ModulePortConnection(
                ModulePort(source, d.at(sourceType).outputs.at(0).name),
                ModulePort(source, d.at(targetType).inputs.at(0).name)
            )
        );
        return true;
    }

    set<string> usedTargetPorts;

    for (const auto& sp : d.at(sourceType).outputs) {
        for (const auto& tp : d.at(targetType).inputs) {
            if (usedTargetPorts.count(tp.name)) continue;

            if(sp.name == tp.name
            || (sp.name == "out" || sp.name == "output") && (tp.name == "in" || tp.name == "input")
            ) {
                rootComponent()->graph.connect(
                    ModulePortConnection(
                        ModulePort(source, sp.name),
                        ModulePort(target, tp.name)
                    )
                );
                usedTargetPorts.insert(tp.name);
            }
        }
    }

    return usedTargetPorts.size() > 0;
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


GraphEditorBundle::GraphEditorBundle(
    PatchEditor& graphEditor,
    const Doc &initialDoc,
    SubValue<PatchDescriptor> & subPatch,
    const string& rootComponent,
    const PatchDescriptor& initialPatch,
    const vector<PadDescription> &inBus,
    const vector<PadDescription> &outBus,
    const LayoutUpdateCallback &layoutUpdateCb
)
    : editor(
        graphEditor
        , rootComponent
        , initialPatch
        , view
        , initialDoc
        , subPatch
        , inBus
        , outBus
        , layoutUpdateCb
    )
{
    addAndMakeVisible(view);
    view.setViewedComponent(&editor, false);
    resized();
}

void GraphEditorBundle::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
}

void GraphEditorBundle::resized()
{
    view.setBoundsRelative(0, 0, 1.0f, 1.0f);
    repaint();
}
