#include "GraphView.h"
#include <algorithm>
#include <utility>

#include "GraphEditor.hpp"

using namespace PhasePhckr;

bool makeModulePoopUp(PopupMenu & poop, const string & moduleName, GfxGraph & gfxGraph){
    Label lbl(moduleName, moduleName);
    lbl.setEditable(true, true, false);
    poop.addCustomItem(1, &lbl, 20, 20, false);
    poop.addItem(2, "delete");
    int choice = poop.show();
    switch (choice){
    case 1:
        gfxGraph.rename(
            moduleName,
            lbl.getText().toStdString()
        );
        return true;
        break;
    case 2:
        gfxGraph.remove(moduleName);
        return true;
        break;
    default:
        break;
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


void createComponent(set<const GfxModule *> & selection, GfxGraph & gfxGraph, Doc & doc, XY& position){
    ConnectionGraphDescriptor cgd;
    set<string> modules;

    // copy over modules and any non-default values
    for (const auto s : selection) {
        cgd.modules.push_back(s->module);
        for (const auto &p : s->inputs) {
            if (p.assignedValue) {
                cgd.values.push_back(ModulePortValue{ s->module.name, p.port, p.value });
            }
        }
        modules.insert(s->module.name);
    }

    set<string> inAlias;
    set<string> outAlias;
    list<ModulePortAlias> inBusConnections;
    list<ModulePortAlias> outBusConnections;

    string name = "newComponent";
    int ctr = 0;
    while (gfxGraph.hasModuleName(name + to_string(ctr))){
        ctr++;
    }
    name += to_string(ctr);

    for (auto &w : gfxGraph.wires) {
        // copy any internal connections
        if (modules.count(w.connection.source.module) && modules.count(w.connection.target.module)) {
            cgd.connections.push_back(w.connection);
        }
        // store the external connections
        else if (modules.count(w.connection.target.module)) {
            auto mp = w.connection.target;
            string alias = mp.port;
            while (inAlias.count(alias)) { alias += "_"; }
            inAlias.insert(alias);
            ModulePortAlias mpa = { alias, { mp } };
            inBusConnections.push_back(mpa);
            // remap connection graph to the new place
            w.connection.target.module = name;
            w.connection.target.port = alias;
        }
        else if (modules.count(w.connection.source.module)) {
            auto mp = w.connection.source;
            string alias = mp.port;
            while (outAlias.count(alias)) { alias += "_"; }
            outAlias.insert(alias);
            ModulePortAlias mpa = { alias,{ mp } };
            outBusConnections.push_back(mpa);
            // remap connection graph to the new place
            w.connection.source.module = name;
            w.connection.source.port = alias;
        }
    }

    // create a ComponentDescriptor
    string type = "@INCOMPONENT";
    ctr = 0;
    while (gfxGraph.components.count(type + to_string(ctr))) {
        ctr++;
    }
    type += to_string(ctr);

    ComponentDescriptor cmp;
    cmp.graph = cgd;
    cmp.docString = "";
    for (const auto i : inBusConnections) {        
        cmp.inputs.push_back(i);
    }
    for (const auto o : outBusConnections) {
        cmp.outputs.push_back(o);
    }

    // store it on model
    gfxGraph.components[type] = cmp;

    // add it to docList
    ModuleDoc cDoc;
    PhasePhckr::ComponentRegister::makeComponentDoc(type, cmp, cDoc, doc);
    doc.add(cDoc);

    // add it to graph
    vector<ModulePortValue> mpv;
    gfxGraph.modules.push_back(GfxModule(
        ModuleVariable{ name, type },
        (position.x - 0.5f*c_NodeSize) / c_GridSize,
        (position.y - 0.5f*c_NodeSize) / c_GridSize,
        doc,
        mpv
    ));

    gfxGraph.recalculateWires(gfxGraph.modules);

}


bool makeModuleSelectionPoopUp(PopupMenu & poop, set<const GfxModule *> & selection, GfxGraph & gfxGraph, Doc & doc, XY& position) {
    poop.addItem(1, "make component");
    poop.addItem(2, "delete");
    int choice = poop.show();
    switch (choice) {
    case 1:
        createComponent(selection, gfxGraph, doc, position);
        deleteSelectedModules(selection, gfxGraph);
        return true;
    case 2:
        deleteSelectedModules(selection, gfxGraph);
        return true;
    default:
        break;
    }
    return false;
}


bool makePortPoopUp(PopupMenu & poop, GfxModule & gfxModule, const string & port){
    float value;
    if(!gfxModule.getValue(port, value)) return false; // error
    Label lbl(port, to_string(value));
    lbl.setEditable(true, true, false);
    poop.addItem(1, port);
    poop.addCustomItem(2, &lbl, 20, 20, false);
    poop.addItem(3, "clear");
    int choice = poop.show();
    switch (choice){
    case 1:
        break;
    case 2:
        gfxModule.setValue(
            port,
            lbl.getText().getFloatValue()
        );
        return true;
        break;
    case 3:
        gfxModule.clearValue(port);
        return true;
        break;
    default:
        break;
    }
    return false;
}


void GraphView::propagateUserModelChange() {
    repaint();

    while (gfxGraphLock.test_and_set(std::memory_order_acquire));
    GfxGraph gfxGraph_cpy = gfxGraph;
    gfxGraphLock.clear(std::memory_order_release);

    PatchDescriptor graph;
    for (const auto &m : gfxGraph_cpy.modules) {
        if (m.module.name == inBus.name || m.module.name == outBus.name) continue;
        if (m.module.type == inBus.type || m.module.type == outBus.type) continue;
        graph.root.modules.emplace_back(m.module);
        for ( const auto &ip : m.inputs){
            if(ip.assignedValue){
                graph.root.values.emplace_back(ModulePortValue{m.module.name, ip.port, ip.value});
            }
        }
    }
    for (const auto &w : gfxGraph_cpy.wires) {
        graph.root.connections.emplace_back(w.connection);
    }

    graph.components = gfxGraph_cpy.components;

    subscribedCGD.set(subscribedCGDhandle, graph);
}


void GraphView::mouseDoubleClick(const MouseEvent & event) {
    XY mousePos((float)event.x, (float)event.y);
    for (auto & m : gfxGraph.modules) {
        if (m.within(mousePos)) {
            graphEditor.push_tab(m.module.name, m.module.type);
        }
    }
}


void GraphView::mouseDown(const MouseEvent & event) {
    viewPort.setScrollOnDragEnabled(true);
    bool modelChanged = false;
    bool userInteraction = false;
    mouseDownPos = XY((float)event.x, (float)event.y);

    for (auto & m : gfxGraph.modules) {
        string port;
        bool inputPort;
        XY position;
        // drag wire between ports
        if (m.withinPort(mouseDownPos, position, port, inputPort)) {
            if(inputPort && event.mods.isRightButtonDown()){
                PopupMenu poop;
                modelChanged = makePortPoopUp(poop, m, port);
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
                    modelChanged = makeModulePoopUp(poop, m.module.name, gfxGraph);
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
    if (!userInteraction) {
        while (gfxGraphLock.test_and_set(memory_order_acquire));
        // disconnect a wire
        if (gfxGraph.disconnect(mouseDownPos, looseWire)) {
            modelChanged = true;
            userInteraction = true;
        }
        gfxGraphLock.clear(memory_order_release);
    }
    if (!userInteraction) {
        // select region start/stop
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
    if (userInteraction) {
        viewPort.setScrollOnDragEnabled(false);
        repaint();
    }

    if (modelChanged) propagateUserModelChange();
}


void GraphView::mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & d){

}


void GraphView::mouseDrag(const MouseEvent & event) {
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


void GraphView::mouseUp(const MouseEvent & event) {
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


void GraphView::mouseMove(const MouseEvent & event) {
}


void GraphView::updateBounds(const pair<XY, XY>& rectangle) {
    updateBounds(rectangle.first, rectangle.second);
}


void GraphView::itemDropped(const SourceDetails & dragSourceDetails){
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


void GraphView::updateBounds(const XY & position, const XY & size){
    auto bounds = getBounds();
    if (bounds.getWidth() < (position.x + size.x)) {
        bounds.setWidth((int)(position.x + size.x) + 10);
    }
    if (bounds.getHeight() < (position.y + size.y)) {
        bounds.setHeight((int)(position.y + size.y) + 10);
    }
    setBounds(bounds);
}


void GraphView::paint (Graphics& g){
    while (gfxGraphLock.test_and_set(memory_order_acquire));

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


void GraphView::setGraph(const PatchDescriptor& graph) {
  while (connectionGraphDescriptorLock.test_and_set(std::memory_order_acquire));
  auto connectionGraphDescriptor = graph;
  for (const auto & c : graph.components) { // HAX
      ModuleDoc d;
      PhasePhckr::ComponentRegister::makeComponentDoc(c.first, c.second, d, doc);
      doc.add(d);
  }
  connectionGraphDescriptorLock.clear(std::memory_order_release);

  connectionGraphDescriptor.root.modules.push_back(inBus);
  connectionGraphDescriptor.root.modules.push_back(outBus);

  const string start = inBus.name;
  const string stop = outBus.name;
  ModulePositionMap modulePositions;
  setNodePositions(connectionGraphDescriptor.root, modulePositions, start, stop);

  // build the render/user interaction model
  updateRenderComponents(connectionGraphDescriptor, modulePositions);

}


void GraphView::updateRenderComponents(
    const PatchDescriptor &cgd_copy,
    const ModulePositionMap & mp
) 
{
    // create the renderable structures

    while (gfxGraphLock.test_and_set(std::memory_order_acquire));

    gfxGraph = GfxGraph();

    gfxGraph.components = cgd_copy.components;

    for (const auto & m : cgd_copy.root.modules) {
        gfxGraph.add(
            m, 
            doc, 
            XY(mp.at(m.name).x, mp.at(m.name).y), 
            cgd_copy.root.values,
            false
        );
    }

    for (const auto & c : cgd_copy.root.connections) {
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

    gfxGraphLock.clear(std::memory_order_release);

    updateBounds(bounds);

}
