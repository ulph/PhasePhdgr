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


bool makeModulePoopUp(PopupMenu & poop, const string & moduleName, const string & moduleType, GfxGraph & gfxGraph){
    Label nameLbl(moduleName, moduleName);
    nameLbl.setEditable(true, true, false);

    Label typeLbl(moduleType, moduleType);
    typeLbl.setEditable(true, true, false);

    int ctr=1;

    const int nameMenuId = ctr++;
    poop.addCustomItem(nameMenuId, &nameLbl, 20, 20, false);

    int typeMenuId = 999;
    if(moduleType.front() == componentMarker){
        typeMenuId = ctr++;
        poop.addCustomItem(typeMenuId, &typeLbl, 20, 20, false);
    }

    const int delMenuId = ctr++;
    poop.addItem(delMenuId, "delete");

    int choice = poop.show();
    if(choice == delMenuId){
        gfxGraph.remove(moduleName);
        return true;
    }

    if(moduleName != nameLbl.getText().toStdString()){
        return gfxGraph.rename(
            moduleName,
            nameLbl.getText().toStdString()
        );
    }
    else if(moduleType != typeLbl.getText().toStdString()){
        return gfxGraph.renameComponent(
            moduleType,
            typeLbl.getText().toStdString()
        );
    }

    return false;
}


void deleteSelectedModules(set<const GfxModule *> & selection, GfxGraph & gfxGraph) {
    list<string> wipe;
    for (const auto s : selection) {
        wipe.push_back(s->module.name);
    }
    selection.clear();
    for (const auto &w : wipe) {
        gfxGraph.remove(w);
    }
}


bool makeModuleSelectionPoopUp(PopupMenu & poop, set<const GfxModule *> & selection, GfxGraph & gfxGraph, Doc & doc, XY& position) {
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
            gfxGraph.createComponentFromSelection(selectedModules, doc, position);
        }
        return true;
    case 2:
        deleteSelectedModules(selection, gfxGraph);
        return true;
    default:
        break;
    }
    return false;
}


bool makePortPoopUp(PopupMenu & poop, GfxModule & gfxModule, const string & port, GfxGraph& gfxGraph, bool inputPort){
    float value = 0.f;
    if(inputPort && !gfxModule.getValue(port, value)) return false; // error

    // TODO deletion of component ports
    // TODO change port units/docstrings

    Label lbl(port+"_v", to_string(value));
    Label nameLbl(port, port);

    if (inputPort) {
        lbl.setEditable(true, true, false);
        poop.addItem(1, port);
        poop.addCustomItem(2, &lbl, 20, 20, false);
        poop.addItem(3, "clear value");
    }

    poop.addItem(4, "disconnect all");

    nameLbl.setEditable(true, true, false);
    if(gfxModule.module.type.front() == componentMarker){
        poop.addCustomItem(5, &nameLbl, 20, 20, false);
        poop.addItem(6, "remove port");
    }

    int choice = poop.show();

    if (inputPort) {
        if (choice == 3) {
            gfxModule.clearValue(port);
            return true;
        }

        if (value != lbl.getText().getFloatValue()) {
            return gfxModule.setValue(
                port,
                lbl.getText().getFloatValue()
            );
        }
    }

    if (choice == 4) {
        return gfxGraph.disconnectPort(
            gfxModule.module.name,
            port,
            inputPort
        );
    }

    if(port != nameLbl.getText()){
        return gfxGraph.renameComponentPort(
            gfxModule.module.type,
            port,
            nameLbl.getText().toStdString(),
            inputPort
        );
    }

    if (choice == 6) {
        return gfxGraph.removeComponentPort(
            gfxModule.module.type,
            port,
            inputPort
        );
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
    while (gfxGraphLock.test_and_set(std::memory_order_acquire));
    patch = gfxGraph.exportModelData();
    gfxGraphLock.clear(std::memory_order_release);

    subPatch.set(subPatchHandle, patch);
    repaint();
}


void GraphEditor::mouseDoubleClick(const MouseEvent & event) {
    XY mousePos((float)event.x, (float)event.y);
    for (auto & m : gfxGraph.modules) {
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

    for (auto & m : gfxGraph.modules) {
        string port;
        bool inputPort;
        XY position;
        // drag wire between ports
        if (m.withinPort(mouseDownPos, position, port, inputPort)) {
            if(event.mods.isRightButtonDown()){
                PopupMenu poop;
                modelChanged = makePortPoopUp(poop, m, port, gfxGraph, inputPort);
            }
            else{
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
                    modelChanged = makeModuleSelectionPoopUp(poop, selectedModules, gfxGraph, doc, mouseDownPos);
                }
                else {
                    modelChanged = makeModulePoopUp(poop, m.module.name, m.module.type, gfxGraph);
                }
            }
            else if (event.mods.isShiftDown()) {
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
        while (gfxGraphLock.test_and_set(memory_order_acquire));
        if (gfxGraph.disconnect(mouseDownPos, looseWire)) {
            modelChanged = true;
            userInteraction = true;
        }
        gfxGraphLock.clear(memory_order_release);
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
    if (draggedModule) {
        auto mousePos = XY((float)event.x, (float)event.y);
        XY delta = mousePos - mouseDownPos;
        mouseDownPos = mousePos;
        draggedModule->position += delta;
        draggedModule->repositionPorts();
        auto mv = vector<GfxModule>{ *draggedModule };
        while (gfxGraphLock.test_and_set(memory_order_acquire));
        gfxGraph.recalculateWires(mv);
        gfxGraphLock.clear(memory_order_release);
        updateBounds(gfxGraph.getBounds());
        repaint();
    }
    if (looseWire.isValid) {
        looseWire.destination.x = (float)event.x;
        looseWire.destination.y = (float)event.y;
        updateBounds(gfxGraph.getBounds());
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
        while (gfxGraphLock.test_and_set(memory_order_acquire));
        modelChanged = gfxGraph.connect(looseWire, mousePos);
        gfxGraphLock.clear(memory_order_release);
    }
    if (selecting) {
        auto selectionRegion = Rectangle<float>(selectionStart, selectionStop);
        for (const auto &m : gfxGraph.modules) {
            if(m.module.name == c_inBus.name || m.module.name == c_outBus.name) continue;
            Rectangle<float> mr(m.position.x, m.position.y, m.size.x, m.size.y);
            if (selectionRegion.intersectRectangle(mr)){
                selectedModules.insert(&m);
            }
        }
    }
    selecting = false;
    looseWire.isValid = false;
    gfxGraph.moveIntoView(); // don't do this continously or stuff gets weird
    repaint();
    if (modelChanged) propagateUserModelChange();
}


void GraphEditor::mouseMove(const MouseEvent & event) {

}


void GraphEditor::updateBounds(const pair<XY, XY>& rectangle) {
    updateBounds(rectangle.first, rectangle.second);
}


void GraphEditor::itemDropped(const SourceDetails & dragSourceDetails){
    while (gfxGraphLock.test_and_set(memory_order_acquire));
    auto thing = dragSourceDetails.description.toString().toStdString();
    auto dropPos = dragSourceDetails.localPosition;
    bool modelChanged = gfxGraph.add(
        thing, 
        doc, 
        XY((float)dropPos.x, (float)dropPos.y)
    );
    gfxGraphLock.clear(memory_order_release);
    if (modelChanged) propagateUserModelChange();
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
    while (gfxGraphLock.test_and_set(memory_order_acquire));

    updateRenderComponents();

    g.fillAll(Colours::black);

    for(auto &w : gfxGraph.wires){
        w.draw(g);
    }
    for (auto &mb : gfxGraph.modules) {
        mb.draw(g, selectedModules.count(&mb));
    }
    if (looseWire.isValid) {
        looseWire.draw(g);
    }
    if (selecting) {
        g.setColour(Colour(0x44ffffff));
        g.fillRect(Rectangle<float>(selectionStart, selectionStop));
    }

    gfxGraphLock.clear(std::memory_order_release);
}


void GraphEditor::setDoc(const Doc& newDoc){
    while (gfxGraphLock.test_and_set(std::memory_order_acquire));

    doc = newDoc;
    auto inBus_ = Doc::makeBusModuleDoc(inBus, true);
    auto outBus_ = Doc::makeBusModuleDoc(outBus, false);
    doc.add(inBus_);
    doc.add(outBus_);

    docIsDirty = true;

    gfxGraphLock.clear(std::memory_order_release);

    repaint();
}


void GraphEditor::setGraph(const PatchDescriptor& newPatch) {
  while (gfxGraphLock.test_and_set(std::memory_order_acquire));

  patch = newPatch;
  patchIsDirty = true;

  gfxGraphLock.clear(std::memory_order_release);

  repaint();
}


void GraphEditor::updateRenderComponents() 
{
    // call from within paint

    if (docIsDirty) {
        gfxGraph.designPorts(doc);
        gfxGraph.recalculateWires(gfxGraph.modules);
        
        docIsDirty = false;
    }

    if (patchIsDirty) {

        patch.root.graph.modules.push_back(c_inBus);
        patch.root.graph.modules.push_back(c_outBus);

        const string start = c_inBus.name;
        const string stop = c_outBus.name;
        ModulePositionMap mp;
        setNodePositions(patch.root.graph, mp, start, stop);

        gfxGraph.components.clear();
        gfxGraph.modules.clear();
        gfxGraph.wires.clear();

        gfxGraph.components = patch.components;

        for (const auto & m : patch.root.graph.modules) {
            XY xy(mp.at(m.name).x, mp.at(m.name).y);
            if (patch.layout.count(m.name)) {
                auto xy_ = patch.layout.at(m.name);
                xy.x = (float)xy_.x / (float)c_GridSize;
                xy.y = (float)xy_.y / (float)c_GridSize;
            }
            gfxGraph.add(
                m,
                doc,
                xy,
                patch.root.graph.values,
                false
            );
        }

        for (const auto & c : patch.root.graph.connections) {
            gfxGraph.connect(c);
        }

        auto bounds = gfxGraph.getBounds();
        XY delta(0, 0);
        if (bounds.first.x < 0) delta.x = -bounds.first.x;
        if (bounds.first.y < 0) delta.y = -bounds.first.y;
        if (delta.x && delta.y) {
            gfxGraph.moveDelta(delta);
            bounds = gfxGraph.getBounds();
        }

        updateBounds(bounds);
        patchIsDirty = false;
    }
    

}
