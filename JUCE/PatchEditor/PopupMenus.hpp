#pragma once

#include <phasephckr.hpp>
#include "JuceHeader.h"

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

class PortPopupMenu {
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

public:

    bool build(PopupMenu& popupMenu, PatchDescriptor & patch, const string& rootComponentName, GfxModule & gfxModule, const string & port_, bool inputPort_)
    {
        popupMenu.clear();

        inputPort = inputPort_;
        port = port_;
        moduleName = gfxModule.module.name;
        moduleType = gfxModule.module.type;

        if (inputPort && !gfxModule.getValue(port, value)) return false; // error

        if (moduleName == c_inBus.name || moduleName == c_outBus.name) {
            if (patch.components.count(rootComponentName)) {
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

        if (patch.components.count(moduleType)) {
            PopupMenu cmpPoop;

            auto& cmp = patch.components[moduleType];

            cmpPoop.addCustomItem(5, &nameLbl, 200, 20, false);

            if (inputPort && 0 == cmp.getPort(port, pd, true)) {
                unitLbl.title.setText("Unit:", NotificationType::dontSendNotification);
                unitLbl.edit.setText(pd.unit, NotificationType::dontSendNotification);

                defaultValueLbl.title.setText("Default:", NotificationType::dontSendNotification);
                defaultValueLbl.edit.setText(to_string(pd.defaultValue), NotificationType::dontSendNotification);

                cmpPoop.addCustomItem(7, &defaultValueLbl, 200, 20, false);
                cmpPoop.addCustomItem(8, &unitLbl, 200, 20, false);
            }
            cmpPoop.addItem(6, "remove port");

            popupMenu.addSubMenu("Component", cmpPoop);
        }

        return true;
    }

    bool handleChoice(PatchDescriptor & patch, ComponentDescriptor* rootComponent, int choice) {
        if (!rootComponent) return false;
        bool validRootComponent = rootComponent == &patch.root;
        if (!validRootComponent) {
            for (const auto& kv : patch.components) {
                if (&(kv.second) == rootComponent) validRootComponent = true;
            }
        }
        if (!validRootComponent) return false;

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

};

//

