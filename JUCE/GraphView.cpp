#include "GraphView.h"
#include <algorithm>
#include <utility>
#include "GraphViewNodeStuff.hpp"

using namespace PhasePhckr;

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
    }
    for (const auto &w : gfxGraph_cpy.wires) {
        graph.connections.emplace_back(w.connection);
    }

    graph.values = valuesCopy;

    subscribedCGD.set(subscribedCGDhandle, graph);
}


void GraphView::mouseDown(const MouseEvent & event) {
    viewPort.setScrollOnDragEnabled(true);
    bool modelChanged = false;
    bool userInteraction = false;
    XY mousePos((float)event.x, (float)event.y);
    while (gfxGraphLock.test_and_set(std::memory_order_acquire));
    for (auto & m : gfxGraph.modules) {
        std::string port;
        bool inputPort;
        XY position;
        // drag wire between ports
        if (m.withinPort(mousePos, position, port, inputPort)) {
            looseWire.isValid = true;
            looseWire.destination = mousePos;
            looseWire.attachedAtSource = !inputPort;
            looseWire.attachedPort = { m.module.name, port };
            looseWire.position = position;
            userInteraction = true;
            break;
        }
        // move a module
        if (m.within(mousePos)) {
            draggedModule = &m;
            userInteraction = true;
            break;
        }
    }
    if (!userInteraction) {
        for (auto wit = gfxGraph.wires.begin(); wit != gfxGraph.wires.end(); ++wit) {
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
                gfxGraph.wires.erase(wit);
                userInteraction = true;
                modelChanged = true;
                break;
            }
        }
    }
    if (userInteraction) {
        viewPort.setScrollOnDragEnabled(false);
        repaint();
    }
    gfxGraphLock.clear(std::memory_order_release);
    if (modelChanged) propagateUserModelChange();
}


void GraphView::mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & d){

}


void GraphView::mouseDrag(const MouseEvent & event) {
    bool modelChanged = false;
    while (gfxGraphLock.test_and_set(std::memory_order_acquire));
    if (draggedModule) {
        draggedModule->position.x = (float)event.x - draggedModule->size.x * 0.5f;
        draggedModule->position.y = (float)event.y - draggedModule->size.y * 0.5f;
        draggedModule->repositionPorts();
        auto mv = std::vector<GfxModule>{ *draggedModule };
        for (auto &w : gfxGraph.wires) {
            w.calculatePath(mv);
        }
        repaint();
    }
    if (looseWire.isValid) {
        looseWire.destination.x = (float)event.x;
        looseWire.destination.y = (float)event.y;
        repaint();
    }
    gfxGraphLock.clear(std::memory_order_release);
    if (modelChanged) propagateUserModelChange();
}


void GraphView::mouseUp(const MouseEvent & event) {
    while (gfxGraphLock.test_and_set(std::memory_order_acquire));
    XY mousePos((float)event.x, (float)event.y);
    bool modelChanged = false;
    draggedModule = nullptr;
    if (looseWire.isValid) {
        for (auto &m : gfxGraph.modules) {
            if (looseWire.attachedAtSource) {
                for (const auto& p : m.inputs) {
                    if (p.within(mousePos)) {
                        ModulePortConnection newCon;
                        newCon.source = looseWire.attachedPort;
                        newCon.target = ModulePort{ m.module.name, p.port };
                        gfxGraph.wires.emplace_back(GfxWire(newCon, gfxGraph.modules));
                        modelChanged = true;
                    }
                }
            }
            else {
                for (const auto& p : m.outputs) {
                    if (p.within(mousePos)) {
                        ModulePortConnection newCon;
                        newCon.source = ModulePort{ m.module.name, p.port };
                        newCon.target = looseWire.attachedPort;
                        gfxGraph.wires.emplace_back(GfxWire(newCon, gfxGraph.modules));
                        modelChanged = true;
                    }
                }
            }
        }
    }
    looseWire.isValid = false;
    repaint();
    gfxGraphLock.clear(std::memory_order_release);
    if (modelChanged) propagateUserModelChange();
}


void GraphView::mouseMove(const MouseEvent & event) {

}


void GraphView::updateBounds(const std::pair<XY, XY>& rectangle) {
    updateBounds(rectangle.first, rectangle.second);
}


void GraphView::updateBounds(const XY & position, const XY & size){
    auto bounds = getBounds();
    if (bounds.getWidth() < (position.x + size.x)) {
        bounds.setWidth((int)(position.x + size.x));
    }
    if (bounds.getHeight() < (position.y + size.y)) {
        bounds.setHeight((int)(position.y + size.y));
    }
    setBounds(bounds);
}


void GraphView::paint (Graphics& g){
    while (gfxGraphLock.test_and_set(std::memory_order_acquire));

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

  valuesCopy = connectionGraphDescriptor.values;
  
  connectionGraphDescriptor.modules.push_back(inBus);
  connectionGraphDescriptor.modules.push_back(outBus);

  const std::string start = inBus.name;
  const std::string stop = outBus.name;
  std::map<std::string, XY> modulePositions;
  setNodePositions(connectionGraphDescriptor, modulePositions, start, stop);

  // build the render/user interaction model
  updateRenderComponents(connectionGraphDescriptor, modulePositions);

}


void GraphView::updateRenderComponents(const ConnectionGraphDescriptor &cgd_copy, const std::map<std::string, XY> & mp) {
    // create the renderable structures

    while (gfxGraphLock.test_and_set(std::memory_order_acquire));

    gfxGraph = GfxGraph();

    for (const auto & m : cgd_copy.modules) {
        gfxGraph.modules.emplace_back(
            GfxModule(
                m, 
                mp.at(m.name).x,
                mp.at(m.name).y,
                doc, 
                cgd_copy.values
            )
        );
    }

    for (const auto & c : cgd_copy.connections) {
        gfxGraph.wires.emplace_back(
            GfxWire(c, gfxGraph.modules)
        );
    }

    auto bounds = gfxGraph.getBounds();
    XY delta(0, 0);
    if (bounds.first.x < 0) delta.x = -bounds.first.x;
    if (bounds.first.y < 0) delta.y = -bounds.first.y;
    if (delta) {
        gfxGraph.moveDelta(delta);
        bounds = gfxGraph.getBounds();
    }

    gfxGraphLock.clear(std::memory_order_release);

    updateBounds(bounds);

}
