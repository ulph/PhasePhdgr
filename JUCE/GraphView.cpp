#include "GraphView.h"
#include <algorithm>
#include <utility>

#include "GraphEditor.hpp"

using namespace PhasePhckr;


bool makeModulePoopUp(PopupMenu & poop, const string & moduleName, GfxGraph & gfxGraph){
    Label lbl(moduleName, moduleName);
    lbl.setEditable(true, true, false);
    poop.addCustomItem(0, &lbl, 20, 20, false);
    poop.addItem(1, "delete");
    int choice = poop.show();
    gfxGraph.rename(
        moduleName,
        lbl.getText().toStdString()
    );
    switch (choice){
    case 0:
        return true;
        break;
    case 1:
        gfxGraph.remove(moduleName);
        return true;
        break;
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
    poop.addItem(0, port);
    poop.addCustomItem(1, &lbl, 20, 20, false);
    poop.addItem(2, "clear");
    int choice = poop.show();
    gfxModule.setValue(
        port,
        lbl.getText().getFloatValue()
    );
    switch (choice){
    case 0:
        break;
    case 1:
        return true;
        break;
    case 2:
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

    ConnectionGraphDescriptor graph;
    for (const auto &m : gfxGraph_cpy.modules) {
        if (m.module.name == inBus.name || m.module.name == outBus.name) continue;
        if (m.module.type == inBus.type || m.module.type == outBus.type) continue;
        graph.modules.emplace_back(m.module);
        for ( const auto &ip : m.inputs){
            if(ip.assignedValue){
                graph.values.emplace_back(ModulePortValue{m.module.name, ip.port, ip.value});
            }
        }
    }
    for (const auto &w : gfxGraph_cpy.wires) {
        graph.connections.emplace_back(w.connection);
    }

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
    XY mousePos((float)event.x, (float)event.y);

    for (auto & m : gfxGraph.modules) {
        string port;
        bool inputPort;
        XY position;
        // drag wire between ports
        if (m.withinPort(mousePos, position, port, inputPort)) {
            if(event.mods.isRightButtonDown()){
                PopupMenu poop;
                makePortPoopUp(poop, m, port);
                modelChanged = true;
            }
            else{
                looseWire.isValid = true;
                looseWire.destination = mousePos;
                looseWire.attachedAtSource = !inputPort;
                looseWire.attachedPort = { m.module.name, port };
                looseWire.position = position;
            }
            userInteraction = true;
            break;
        }
        // interract with a module
        else if (m.within(mousePos)) {
            if(event.mods.isRightButtonDown()){
                PopupMenu poop;
                makeModulePoopUp(poop, m.module.name, gfxGraph);
                modelChanged = true;
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
        if (gfxGraph.disconnect(mousePos, looseWire)) {
            modelChanged = true;
            userInteraction = true;
        }
        gfxGraphLock.clear(memory_order_release);
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
        draggedModule->position.x = (float)event.x - draggedModule->size.x * 0.5f;
        draggedModule->position.y = (float)event.y - draggedModule->size.y * 0.5f;
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
        mb.draw(g);
    }
    if (looseWire.isValid) {
        looseWire.draw(g);
    }

    gfxGraphLock.clear(std::memory_order_release);
}


void GraphView::setGraph(const ConnectionGraphDescriptor& graph) {
  while (connectionGraphDescriptorLock.test_and_set(std::memory_order_acquire));
  auto connectionGraphDescriptor = graph;
  connectionGraphDescriptorLock.clear(std::memory_order_release);

  connectionGraphDescriptor.modules.push_back(inBus);
  connectionGraphDescriptor.modules.push_back(outBus);

  const string start = inBus.name;
  const string stop = outBus.name;
  ModulePositionMap modulePositions;
  setNodePositions(connectionGraphDescriptor, modulePositions, start, stop);

  // build the render/user interaction model
  updateRenderComponents(connectionGraphDescriptor, modulePositions);

}


void GraphView::updateRenderComponents(
    const ConnectionGraphDescriptor &cgd_copy, 
    const ModulePositionMap & mp
) 
{
    // create the renderable structures

    while (gfxGraphLock.test_and_set(std::memory_order_acquire));

    gfxGraph = GfxGraph();

    for (const auto & m : cgd_copy.modules) {
        gfxGraph.add(
            m, 
            doc, 
            XY(mp.at(m.name).x, mp.at(m.name).y), 
            cgd_copy.values,
            false
        );
    }

    for (const auto & c : cgd_copy.connections) {
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
