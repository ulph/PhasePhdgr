#pragma once

#include "Utils.hpp"
#include "phasephckr.hpp"
#include "PPGrid.h"

using namespace PhasePhckr;

#define shutup NotificationType::dontSendNotification

class SettingsEditor : public Component, public ButtonListener {
private:
    PresetSettings settings;
    int subSettingsHandle = -1;
    SubValue<PresetSettings> &subSettings;

    PPGrid grid;

    PPGGrid polyGrid;
    TextButton polyphony1;
    TextButton polyphony2;
    TextButton polyphony4;
    TextButton polyphony8;
    TextButton polyphony16;
    TextButton polyphony32;
    TextButton polyphony64;

    PPGGrid stealGrid;
    TextButton steal0;
    TextButton steal1;
    TextButton steal2;
    TextButton steal3;
    TextButton steal4;
    TextButton steal5;

    PPGGrid reactivateGrid;
    TextButton reactivate0;
    TextButton reactivate1;
    TextButton reactivate2;
    TextButton reactivate3;
    TextButton reactivate4;

    PPGGrid legatoGrid;
    TextButton legato0;
    TextButton legato1;
    TextButton legato2;
    TextButton legato3;

    PPGGrid activateGrid;
    TextButton activate0;
    TextButton activate1;

    void applySettings();

public:
    virtual ~SettingsEditor() {
        subSettings.unsubscribe(subSettingsHandle);
    }

    SettingsEditor(SubValue<PresetSettings>& subSettings_);

    void resized() override {
        grid.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    }

    virtual void buttonClicked(Button* b) override;

};
