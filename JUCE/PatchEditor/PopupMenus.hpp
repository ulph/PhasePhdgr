#pragma once

// TODO, this is a hot pile of garbage

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "CommonEditor.hpp"

using namespace std;
using namespace PhasePhckr;

class TextLabelMenuEntry : public Component {
public:
    Label title;
    Label edit;
    TextLabelMenuEntry() {
        addAndMakeVisible(title);
        addAndMakeVisible(edit);
        edit.setEditable(true, true, false);
    }
    void resized() override {
        title.setBoundsRelative(0, 0, 0.25, 1);
        edit.setBoundsRelative(0.25, 0, 0.75, 1);
    }
};

class PopupMenuCallback : public ModalComponentManager::Callback {
private:
    function<void(int)> callback;
public:
    PopupMenuCallback(const function<void(int)> & cb)
        : callback(cb) {}
    virtual void modalStateFinished(int returnValue) override {
        callback(returnValue);
    }
};

class PortPopupMenuData {
private:
    TextLabelMenuEntry valueLbl;
    TextLabelMenuEntry defaultValueLbl;
    TextLabelMenuEntry unitLbl;
    PadDescription pd;
    TextLabelMenuEntry nameLbl;
    bool busModule = false;
    bool inputPort = false;
    string moduleName;
    string moduleType;
    string port;
    float value = 0.0f;

    void reset() {
        busModule = false;
        value = 0.0f;
        pd = PadDescription();
        // rest is set in build
    }

public:

    bool build(PopupMenu& popupMenu, PatchDescriptor & patch, ComponentDescriptor* rootComponent, const string& rootComponentName, GfxModule & gfxModule, const string & port_, bool inputPort_)
    {
        popupMenu.clear();
        reset();

        inputPort = inputPort_;
        port = port_;
        moduleName = gfxModule.module.name;
        moduleType = gfxModule.module.type;

        if (inputPort && !gfxModule.getValue(port, value)) return false; // error

        if (moduleName == c_inBus.name || moduleName == c_outBus.name) {
            if (patch.componentBundle.has(rootComponentName)) {
                moduleType = rootComponentName; // hijack
                inputPort = moduleName == c_inBus.name; // flip
                busModule = true;
            }
            else return false;
        }

        valueLbl.title.setText("Value:", NotificationType::dontSendNotification);
        valueLbl.edit.setText(to_string(value), NotificationType::dontSendNotification);

        unitLbl.edit.setText(pd.unit, NotificationType::dontSendNotification);
        defaultValueLbl.edit.setText(to_string(pd.defaultValue), NotificationType::dontSendNotification);

        nameLbl.title.setText("Name:", NotificationType::dontSendNotification);
        nameLbl.edit.setText(port, NotificationType::dontSendNotification);

        if (!busModule) {
            popupMenu.addItem(1, gfxModule.module.name + ":" + port);
        }
        else {
            popupMenu.addItem(1, moduleType + ":" + port);
        }

        if (inputPort && !busModule) {
            popupMenu.addCustomItem(2, &valueLbl, 200, 20, false);
            popupMenu.addItem(3, "clear value");
        }

        popupMenu.addItem(4, "disconnect all");

        if (patch.componentBundle.has(moduleType)) {
            PopupMenu cmpPoop;

            auto& cmp = patch.componentBundle.get(moduleType);

            cmpPoop.addCustomItem(5, &nameLbl, 200, 20, false);

            if (0 == cmp.getPort(port, pd, inputPort)) {
                unitLbl.title.setText("Unit:", NotificationType::dontSendNotification);
                unitLbl.edit.setText(pd.unit, NotificationType::dontSendNotification);
                cmpPoop.addCustomItem(7, &unitLbl, 200, 20, false);
            }

            if (inputPort && 0 == cmp.getPort(port, pd, true)) {
                defaultValueLbl.title.setText("Default:", NotificationType::dontSendNotification);
                defaultValueLbl.edit.setText(to_string(pd.defaultValue), NotificationType::dontSendNotification);
                cmpPoop.addCustomItem(8, &defaultValueLbl, 200, 20, false);
            }

            cmpPoop.addItem(6, "remove port");

            popupMenu.addSubMenu("Component", cmpPoop);
        }

        // dynamic choices ... take care
        {
            int ctr = 9;
            PopupMenu disconnectSubmenu;
            for (int i = 0; i < rootComponent->graph.connections.size(); i++) {
                const auto& c = rootComponent->graph.connections.at(i);
                if (inputPort_ && c.target.module == gfxModule.module.name && c.target.port == port) {
                    disconnectSubmenu.addItem(ctr, c.source.module + " : " + c.source.port);
                }
                else if (!inputPort_ && c.source.module == gfxModule.module.name && c.source.port == port) {
                    disconnectSubmenu.addItem(ctr, c.target.module + " : " + c.target.port);
                }
                ctr++;
            }
            popupMenu.addSubMenu("disconnect", disconnectSubmenu);
        }

        return true;
    }

    bool handleChoice(PatchDescriptor & patch, ComponentDescriptor* rootComponent, int choice) {
        if (!validRootComponent(&patch, rootComponent)) return false;

        const ModulePort mp(moduleName, port);

        if (inputPort && !busModule) {
            if (choice == 3) {
                return 0 == rootComponent->graph.clearValue(mp);
            }

            auto newValue = valueLbl.edit.getText().getFloatValue();
            if (value != newValue) {
                return 0 == rootComponent->graph.setValue(mp, newValue);
            }
        }

        if (choice == 4) {
            return 0 == rootComponent->graph.disconnect(ModulePort(moduleName, port), inputPort);
        }

        auto newPort = nameLbl.edit.getText().toStdString();
        if (port != newPort) {
            return 0 == patch.componentBundle.renamePort(&patch.root, moduleType, port, newPort, inputPort);
        }

        if (choice == 6) {
            return 0 == patch.componentBundle.removePort(moduleType, port, inputPort);
        }

        if (choice >= 9) {
            int i = choice - 9;
            if (i < rootComponent->graph.connections.size()) {
                return 0 == rootComponent->graph.disconnect(rootComponent->graph.connections.at(i));
            }
        }

        auto newUnit = unitLbl.edit.getText().toStdString();
        if (pd.unit != newUnit) {
            return 0 == patch.componentBundle.setPortUnit(moduleType, port, newUnit, inputPort);
        }

        auto newDefault = defaultValueLbl.edit.getText().getFloatValue();
        if (pd.defaultValue != newDefault) {
            return 0 == patch.componentBundle.setPortValue(moduleType, port, newDefault);
        }

        return false;
    }

};

//

const struct ComponentMenuStrings {
    const string createInput = "create input";
    const string createOutput = "create output";
    const string removeConflict = "remove conflicting Component definition";
    const string removeLocal = "(!) remove local Component definition";
    const string createLocal = "create local Component definition";
    const string clone = "change into new Component";
    const string docString = "change docstring";
} c_componentMenuStrings;

struct ComponentPopupMenuState {
    int typeMenuId = 999;
    TextLabelMenuEntry name;
    int createInputMenuId = 999;
    int createOutputMenuId = 999;
    int removeConflictingComponentMenuId = 999;
    int removeLocalComponentMenuId = 999;
    int addLocalComponentMenuId = 999;
    int docStringMenuId = 999;
    TextEditor docStringEditor;
};

void makeComponentPopupMenu(
    PopupMenu & poop,
    int& ctr,
    ComponentPopupMenuState& ids,
    const string& type,
    const PatchDescriptor& patch,
    const map<string, ComponentDescriptor>& global,
    const map<string, ComponentDescriptor>& local
);

bool applyComponentPopuMenuChoice(
    int choice,
    const ComponentPopupMenuState& ids,
    const string& type,
    PatchDescriptor& patch,
    const map<string, ComponentDescriptor>& global
);


class ModulePopupMenuData {
private:
    int cloneComponentMenuId = 0;
    int addComponentInputMenuId = 0;
    int addComponentOutputMenuId = 0;
    int nameMenuId = 0;
    int delMenuId = 0;

    TextLabelMenuEntry nameLbl;
    ComponentPopupMenuState cmpState;

    string moduleName;
    string moduleType;

public:
    bool build(PopupMenu& popupMenu, const PatchDescriptor & patch, const map<string, ComponentDescriptor>& globalComponents, bool validModule, const string& moduleName_, const string& moduleType_, const string& rootComponentName) {

        moduleName = moduleName_;
        moduleType = moduleType_;

        bool isBusModule = (moduleName == c_inBus.name || moduleName == c_outBus.name);
        if (rootComponentName == rootMarker && isBusModule) return false;

        bool isComponentBus = rootComponentName != rootMarker && isBusModule;
        bool isComponent = moduleType.front() == componentMarker || isComponentBus;

        if (!isComponentBus) {
            if (!validModule) return false;
        }
        else {
            if (!validModule && !(isBusModule)) return false;
        }

        nameLbl.title.setText("Name:", NotificationType::dontSendNotification);
        nameLbl.edit.setText(moduleName, NotificationType::dontSendNotification);
        nameLbl.edit.setEditable(true, true, false);

        int ctr = 1;

        nameMenuId = ctr++;
        delMenuId = ctr++;
        if (moduleName != c_inBus.name && moduleName != c_outBus.name) {
            popupMenu.addCustomItem(nameMenuId, &nameLbl, 200, 20, false);
            popupMenu.addItem(delMenuId, "remove module");
        }

        PopupMenu cmpPoop;

        if (isComponent && !isComponentBus) {
            cloneComponentMenuId = ctr++;
            cmpPoop.addItem(cloneComponentMenuId, c_componentMenuStrings.clone);

            makeComponentPopupMenu(cmpPoop, ctr, cmpState, moduleType, patch, globalComponents, patch.componentBundle.getAll());

        }
        else if (isComponentBus) {
            if (moduleName == c_inBus.name) {
                addComponentInputMenuId = ctr++;
                cmpPoop.addItem(addComponentInputMenuId, c_componentMenuStrings.createInput);
            }
            else if (moduleName == c_outBus.name) {
                addComponentOutputMenuId = ctr++;
                cmpPoop.addItem(addComponentOutputMenuId, c_componentMenuStrings.createOutput);
            }
        }

        if (isComponent) popupMenu.addSubMenu("Component", cmpPoop);

        return true;
    }

    bool handleChoice(PatchDescriptor & patch, ComponentDescriptor* rootComponent, const map<string, ComponentDescriptor>& globalComponents, int choice) {
        if (!validRootComponent(&patch, rootComponent)) return false;

        if (applyComponentPopuMenuChoice(choice, cmpState, moduleType, patch, globalComponents)) return true;

        if (choice == 0) {
            auto newModuleName = nameLbl.edit.getText().toStdString();
            if (moduleName != newModuleName) {
                if (0 == rootComponent->graph.rename(moduleName, newModuleName)) {
                    return true;
                }
            }
            return false;
        }

        if (choice == delMenuId) {
            return 0 == rootComponent->graph.remove(moduleName);
        }
        else if (choice == cloneComponentMenuId) {
            ComponentDescriptor cd;
            if (patch.componentBundle.has(moduleType)) cd = patch.componentBundle.get(moduleType);
            else if (globalComponents.count(moduleType)) cd = globalComponents.at(moduleType);
            else return false;
            string newType = moduleType + "_Clone";
            if (0 != patch.componentBundle.add(newType, cd, true)) return false;
            if (!rootComponent->graph.modules.count(moduleName)) return true;
            rootComponent->graph.modules[moduleName] = newType;
            return true;
        }
        else if (choice == addComponentInputMenuId || choice == addComponentOutputMenuId) {
            auto * comp = rootComponent;
            if (comp == nullptr) return false;
            return 0 == comp->addPort("newPort", choice == addComponentInputMenuId, "", 0.0f);
        }

        return false;
    }

};
