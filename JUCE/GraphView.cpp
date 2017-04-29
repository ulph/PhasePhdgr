#include "GraphView.h"
#include <algorithm>
#include <utility>

typedef std::map<std::string, std::set<std::string>> ConnectionsMap;

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
    parent.setScrollOnDragEnabled(true);
    bool modelChanged = false;
    bool userInteraction = false;
    XY mousePos(event.x, event.y);
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
        if (m.within(XY(event.x, event.y))) {
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
        parent.setScrollOnDragEnabled(false);
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
        draggedModule->position.x = event.x - draggedModule->size.x * 0.5;
        draggedModule->position.y = event.y - draggedModule->size.y * 0.5;
        draggedModule->repositionPorts();
        auto mv = std::vector<GfxModule>{ *draggedModule };
        for (auto &w : gfxGraph.wires) {
            w.calculatePath(mv);
        }
        repaint();
    }
    if (looseWire.isValid) {
        looseWire.destination.x = event.x;
        looseWire.destination.y = event.y;
        repaint();
    }
    gfxGraphLock.clear(std::memory_order_release);
    if (modelChanged) propagateUserModelChange();
}


void GraphView::mouseUp(const MouseEvent & event) {
    while (gfxGraphLock.test_and_set(std::memory_order_acquire));
    bool modelChanged = false;
    draggedModule = nullptr;
    if (looseWire.isValid) {
        for (auto &m : gfxGraph.modules) {
            if (looseWire.attachedAtSource) {
                for (const auto& p : m.inputs) {
                    if (p.within(XY(event.x, event.y))) {
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
                    if (p.within(XY(event.x, event.y))) {
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
        bounds.setWidth(position.x + size.x);
    }
    if (bounds.getHeight() < (position.y + size.y)) {
        bounds.setHeight(position.y + size.y);
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


void updateNodesY(
    const std::string & node,
    std::map<std::string, XY> & positions,
    const ConnectionsMap & connections,
    int depth,
    const std::string & terminator
)
{
    int currentDepth = positions[node].y;
    if (depth <= currentDepth) return;
    positions[node].y = depth;
    if (node == terminator) return;
    for (const auto &c : connections.at(node)) {
        updateNodesY(c, positions, connections, depth - 1, terminator);
    }
}


void GraphView::setGraph(const ConnectionGraphDescriptor& graph) {
  while (connectionGraphDescriptorLock.test_and_set(std::memory_order_acquire));
  ConnectionGraphDescriptor connectionGraphDescriptor = graph;
  connectionGraphDescriptorLock.clear(std::memory_order_release);

  valuesCopy = connectionGraphDescriptor.values;

  std::map<std::string, XY> modulePositions;

  float halfSreen = getWidth() * 0.5;

  connectionGraphDescriptor.modules.push_back(inBus);
  connectionGraphDescriptor.modules.push_back(outBus);
  auto cgd = connectionGraphDescriptor;

  // (convinience) store the connections between nodes
  ConnectionsMap connections;
  for (const auto &mpc : cgd.connections) {
      connections[mpc.source.module].insert(mpc.target.module);
      connections[mpc.target.module].insert(mpc.source.module);
  }

  // initial positions
  for(const auto &mv : cgd.modules){
    modulePositions[mv.name] = XY(0, INT_MIN);
  }

  const std::string start = inBus.name;
  const std::string stop = outBus.name;
  modulePositions[start] = XY(0, INT_MIN);
  modulePositions[stop] = XY(0, INT_MIN);

  // find all paths starting from inBus
  int depth = 0;
  updateNodesY(
      stop, modulePositions, connections, depth, start
  );
  
  // special case to make inBus always at the top
  for (auto const &p: modulePositions){
      if(p.first != start && p.second.y <= modulePositions[start].y){
          modulePositions[start].y = p.second.y - 1;
      }
  }

  // and shift everything to positive
  int y_bias = modulePositions[start].y;
  for (auto &p: modulePositions){
      p.second.y -= y_bias;
  }

  // TODO -- traverse X as well

  updateRenderComponents(cgd, modulePositions);

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
