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
                NYI;
//                modelChanged = makePortPoopUp(poop, m, port, gfxGraph, inputPort);
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
                    NYI;
//                    modelChanged = makeModuleSelectionPoopUp(poop, selectedModules, gfxGraph, doc, mouseDownPos);
                }
                else {
                    NYI;
//                    modelChanged = makeModulePoopUp(poop, m.module.name, m.module.type, gfxGraph);
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
        if (gfxGraph.disconnect(mouseDownPos, looseWire)) {
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
    if (draggedModule) {
        auto mousePos = XY((float)event.x, (float)event.y);
        XY delta = mousePos - mouseDownPos;
        mouseDownPos = mousePos;
        draggedModule->position += delta;
        draggedModule->repositionPorts();
        auto mv = vector<GfxModule>{ *draggedModule };

        gfxGraphLock.lock();
        gfxGraph.recalculateWires();
        gfxGraphLock.unlock();

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
        auto l = gfxGraphLock.make_scoped_lock();
        modelChanged = gfxGraph.connect(looseWire, mousePos);
    }
    if (selecting) {
        auto l = gfxGraphLock.make_scoped_lock();
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
    XY mousePos((float)event.x, (float)event.y);

    auto l = gfxGraphLock.make_scoped_lock();

    // replace with something less terrible that doesn't search the whole bloody graph every pixel

    for (auto& m : gfxGraph.modules) {
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

    for (auto wit = gfxGraph.wires.begin(); wit != gfxGraph.wires.end(); ++wit) {
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


void GraphEditor::updateBounds(const pair<XY, XY>& rectangle) {
    updateBounds(rectangle.first, rectangle.second);
}


void GraphEditor::itemDropped(const SourceDetails & dragSourceDetails){
    bool modelChanged = false;

    gfxGraphLock.lock();

    auto thing = dragSourceDetails.description.toString().toStdString();
    auto dropPos = dragSourceDetails.localPosition;
    NYI; //add module

    gfxGraphLock.unlock();

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
    auto scoped_lock = gfxGraphLock.make_scoped_lock();

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
        gfxGraph.designPorts(doc);
        gfxGraph.recalculateWires();
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
