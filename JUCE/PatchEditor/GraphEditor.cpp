#include "GraphEditor.hpp"
#include <algorithm>
#include <utility>

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
                rootComponent()->layout.insert_or_assign(newModuleName, v);
            }
            return true;
        }
        else return false;
    }

    return false;
}

bool GraphEditor::makeModuleSelectionPoopUp(PopupMenu &poop, set<const GfxModule *> &selection, XY &position) {
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
        return 0 == patch.createNewComponentType(rootComponent(), selectedModules, newType);
    }
    return true;
    case 2:
        for (const auto* m : selection) {
            rootComponent()->graph.remove(m->module.name);
        }
        return true;
    default:
        break;
    }
    return false;
}

bool GraphEditor::makePortPoopUp(PopupMenu & poop, GfxModule & gfxModule, const string & port, bool inputPort) {
    auto moduleName = gfxModule.module.name;
    auto moduleType = gfxModule.module.type;
    float value = 0.f;
    bool busModule = false;
    if (inputPort && !gfxModule.getValue(port, value)) return false; // error

    if (moduleName == c_inBus.name || moduleName == c_outBus.name) {
        if (patch.components.count(rootComponentName)) {
            moduleType = rootComponentName; // hijack
            inputPort = moduleName == c_inBus.name; // flip
            busModule = true;
        }
        else return false;
    }
 
    const ModulePort mp(moduleName, port);
    
    TextLabelMenuEntry valueLbl;
    valueLbl.title.setText("Value:", NotificationType::dontSendNotification);
    valueLbl.edit.setText(to_string(value), NotificationType::dontSendNotification);

    TextLabelMenuEntry defaultValueLbl;
    TextLabelMenuEntry unitLbl;
    PadDescription pd;
    unitLbl.edit.setText(pd.unit, NotificationType::dontSendNotification);
    defaultValueLbl.edit.setText(to_string(pd.defaultValue), NotificationType::dontSendNotification);

    TextLabelMenuEntry nameLbl;
    nameLbl.title.setText("Name:", NotificationType::dontSendNotification);
    nameLbl.edit.setText(port, NotificationType::dontSendNotification);

    if (!busModule) {
        poop.addItem(1, gfxModule.module.name + ":" + port);
    }
    else {
        poop.addItem(1, moduleType + ":" + port);
    }

    if (inputPort && !busModule) {
        poop.addCustomItem(2, &valueLbl, 200, 20, false);
        poop.addItem(3, "clear value");
    }

    poop.addItem(4, "disconnect all");

    if (patch.components.count(moduleType)) {
        PopupMenu cmpPoop;

        auto& cmp = patch.components[moduleType];

        cmpPoop.addCustomItem(5, &nameLbl, 200, 20, false);

        if (inputPort && 0 == cmp.getPort(port, pd, true)){
            unitLbl.title.setText("Unit:", NotificationType::dontSendNotification);
            unitLbl.edit.setText(pd.unit, NotificationType::dontSendNotification);

            defaultValueLbl.title.setText("Default:", NotificationType::dontSendNotification);
            defaultValueLbl.edit.setText(to_string(pd.defaultValue), NotificationType::dontSendNotification);

            cmpPoop.addCustomItem(7, &defaultValueLbl, 200, 20, false);
            cmpPoop.addCustomItem(8, &unitLbl, 200, 20, false);
        }
        cmpPoop.addItem(6, "remove port");

        poop.addSubMenu("Component", cmpPoop);
    }

    int choice = poop.show();

    if (inputPort && !busModule) {
        if (choice == 3) {
            return 0 == rootComponent()->graph.clearValue(mp);
        }

        auto newValue = valueLbl.edit.getText().getFloatValue();
        if (value != newValue) {
            return 0 == rootComponent()->graph.setValue(mp, newValue);
        }
    }

    if (choice == 4) {
        return 0 == rootComponent()->graph.disconnect(ModulePort(moduleName, port), inputPort);
    }

    auto newPort = nameLbl.edit.getText().toStdString();
    if (port != newPort) {
        if (!patch.components.count(moduleType)) return false;
        return 0 == patch.renameComponentTypePort(moduleType, port, newPort, inputPort);
    }

    if (choice == 6) {
        if (!patch.components.count(moduleType)) return false;
        auto& comp = patch.components.at(moduleType);
        return 0 == comp.removePort(port, inputPort);
    }

    auto newUnit = unitLbl.edit.getText().toStdString();
    if (pd.unit != newUnit) {
        if (!patch.components.count(moduleType)) return false;
        auto& comp = patch.components.at(moduleType);
        return 0 == comp.changePortUnit(port, newUnit);
    }

    auto newDefault = defaultValueLbl.edit.getText().getFloatValue();
    if (pd.defaultValue != newDefault) {
        if (!patch.components.count(moduleType)) return false;
        auto& comp = patch.components.at(moduleType);
        return 0 == comp.changePortValue(port, newDefault);
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
    const string& rootComponent,
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
    , rootComponentName(rootComponent)
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
    // TODO, really threadsafe??
    updateLayout();
    subPatch.set(-1, patch); // we want it back (lazily forces refresh/sync)
    repaint();
}

void GraphEditor::propagateLayout() {
    // TODO, really threadsafe??
    updateLayout();
    layoutUpdateCallback(rootComponentName, rootComponent()->layout);
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
                modelChanged = makePortPoopUp(poop, m, port, inputPort);
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
                    modelChanged = makeModuleSelectionPoopUp(poop, selectedModules, mouseDownPos);
                }
                else {
                    modelChanged = makeModulePoopUp(poop, m.module.name, m.module.type);
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

    if (modelChanged) propagatePatch();
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
        rootComponent()->layout.insert_or_assign(
            draggedModule->module.name,
            ModulePosition(draggedModule->position.x, draggedModule->position.y)
        );
        recalculateWires(mv);
        gfxGraphLock.unlock();
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
        int mX = event.position.getX();
        int mY = event.position.getY();

        int dx = 0;
        int dy = 0;
        
        auto b = viewPort.getViewArea();//.transformed(getTransform());

        int bX = b.getX();
        int bY = b.getY();
        int bW = b.getWidth();
        int bH = b.getHeight();

        if (bX > mX) dx = bX - mX;
        if (bY > mY) dy = bY - mY;
        if (bX+bW < mX) dx = (bX + bW) - mX;
        if (bY+bH < mY) dy = (bY + bH) - mY;

        if ( abs(dx) > 2 || abs(dy) > 2 ){
            setTopLeftPosition(getX() + dx*0.5f, getY() + dy*0.5f);
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
    if (modelChanged) propagatePatch();
    else if (draggedModule) propagateLayout();
    draggedModule = nullptr;
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

        while (rootComponent()->graph.add(fullName, thing)) {
            if (!moduleNameIsValid(fullName)) return;
            i++;
            fullName = name + to_string(i);
        }

        rootComponent()->layout[fullName] = ModulePosition(
            (float)dropPos.x - 0.5f*c_NodeSize,
            (float)dropPos.y - 0.5f*c_NodeSize
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

    ModuleDoc inBus_ = Doc::makeBusModuleDoc(inBus, true);
    ModuleDoc outBus_ = Doc::makeBusModuleDoc(outBus, false);

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
                xy.x = (float)xy_.x / (float)c_GridSize;
                xy.y = (float)xy_.y / (float)c_GridSize;
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

bool GraphEditor::disconnect(const XY& mousePos, GfxLooseWire &looseWire) {
    for (auto wit = wires.begin(); wit != wires.end(); ++wit) {
        bool nearestSource = false;
        // disconnect a wire
        if (wit->within(mousePos, nearestSource)) {
            auto ret = rootComponent()->graph.disconnect(wit->connection);
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
                    return 0 == rootComponent()->graph.connect(
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
                    return 0 == rootComponent()->graph.connect(
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
