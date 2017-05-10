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
    vector<PadDescription> inBus;
    vector<PadDescription> outBus;

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
            PadDescription pd = { alias, "", 0};
            inBus.push_back(pd);
            // store 'api' connection
            cgd.connections.push_back(ModulePortConnection{c_inBus.name, alias, w.connection.target.module, w.connection.target.port});
            // remap external connection graph
            w.connection.target.module = name; // deliberately _not_ routing to .inBus
            w.connection.target.port = alias;
        }
        else if (modules.count(w.connection.source.module)) {
            auto mp = w.connection.source;
            string alias = mp.port;
            while (outAlias.count(alias)) { alias += "_"; }
            outAlias.insert(alias);
            PadDescription pd = { alias, "", 0};
            outBus.push_back(pd);
            // store 'api' connectino
            cgd.connections.push_back(ModulePortConnection{w.connection.source.module, w.connection.source.port, c_outBus.name, alias});
            // remap external connection graph
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
    cmp.inBus = inBus;
    cmp.outBus = outBus;

    // store it on model
    gfxGraph.components[type] = cmp;

    // add it to docList
    ModuleDoc cDoc;
    PhasePhckr::ComponentRegister::makeComponentDoc(type, cmp, cDoc);
    doc.add(cDoc);

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

    deleteSelectedModules(selection, gfxGraph); // do this before adding or strange shit happens

    gfxGraph.modules.push_back(gfxM);

    gfxGraph.recalculateWires(gfxGraph.modules);
}


bool makeModuleSelectionPoopUp(PopupMenu & poop, set<const GfxModule *> & selection, GfxGraph & gfxGraph, Doc & doc, XY& position) {
    poop.addItem(1, "make component");
    poop.addItem(2, "delete");
    int choice = poop.show();
    switch (choice) {
    case 1:
        createComponent(selection, gfxGraph, doc, position);
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
        if (m.module.name == "inBus" || m.module.name == "outBus") continue;
        graph.root.graph.modules.emplace_back(m.module);
        for ( const auto &ip : m.inputs){
            if(ip.assignedValue){
                graph.root.graph.values.emplace_back(ModulePortValue{m.module.name, ip.port, ip.value});
            }
        }
        graph.layout.emplace(m.module.name, ModulePosition{(int)m.position.x, (int)m.position.y});
    }
    for (const auto &w : gfxGraph_cpy.wires) {
        graph.root.graph.connections.emplace_back(w.connection);
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
  PhasePhckr::ComponentRegister().makeComponentDocs(doc); // HAX 1
  for (const auto & c : graph.components) {   // HAX 2
      ModuleDoc d;
      PhasePhckr::ComponentRegister::makeComponentDoc(c.first, c.second, d);
      doc.add(d);
  }
  connectionGraphDescriptorLock.clear(std::memory_order_release);

  auto inBus_ = Doc::makeBusModuleDoc(inBus, true);
  auto outBus_ = Doc::makeBusModuleDoc(outBus, false);
  doc.add(inBus_);
  doc.add(outBus_);

  connectionGraphDescriptor.root.graph.modules.push_back(c_inBus);
  connectionGraphDescriptor.root.graph.modules.push_back(c_outBus);

  const string start = c_inBus.name;
  const string stop = c_outBus.name;
  ModulePositionMap modulePositions;
  setNodePositions(connectionGraphDescriptor.root.graph, modulePositions, start, stop);

  // build the render/user interaction model
  updateRenderComponents(connectionGraphDescriptor, modulePositions);

}


void GraphView::updateRenderComponents(
    const PatchDescriptor &cgd_copy,
    const ModulePositionMap & mp
) 
{
    // create the renderable structures

    GfxGraph newGfxGraph = GfxGraph();

    newGfxGraph.components = cgd_copy.components;

    for (const auto & m : cgd_copy.root.graph.modules) {
        XY xy(mp.at(m.name).x, mp.at(m.name).y);
        if (cgd_copy.layout.count(m.name)) {
            auto xy_ = cgd_copy.layout.at(m.name);
            xy.x = (float)xy_.x / (float)c_GridSize;
            xy.y = (float)xy_.y / (float)c_GridSize;
        }
        newGfxGraph.add(
            m, 
            doc, 
            xy, 
            cgd_copy.root.graph.values,
            false
        );
    }

    for (const auto & c : cgd_copy.root.graph.connections) {
        newGfxGraph.connect(c);
    }

    auto bounds = newGfxGraph.getBounds();
    XY delta(0, 0);
    if (bounds.first.x < 0) delta.x = -bounds.first.x;
    if (bounds.first.y < 0) delta.y = -bounds.first.y;
    if (delta.x && delta.y) {
        newGfxGraph.moveDelta(delta);
        bounds = newGfxGraph.getBounds();
    }

    while (gfxGraphLock.test_and_set(std::memory_order_acquire));
    gfxGraph = newGfxGraph;
    gfxGraphLock.clear(std::memory_order_release);

    updateBounds(bounds);

}
