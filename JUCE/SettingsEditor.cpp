#include "SettingsEditor.hpp"

void SettingsEditor::applySettings(){
    switch (settings.polyphony) {
    case 1: polyphony1.setToggleState(true, shutup); break;
    case 2: polyphony2.setToggleState(true, shutup); break;
    case 4: polyphony4.setToggleState(true, shutup); break;
    case 8: polyphony8.setToggleState(true, shutup); break;
    case 16: polyphony16.setToggleState(true, shutup); break;
    case 32: polyphony32.setToggleState(true, shutup); break;
    case 64: polyphony64.setToggleState(true, shutup); break;
    default: PP_NYI; break;
    }

    switch (settings.noteStealPolicy) {
    case NoteStealPolicyDoNotSteal: steal0.setToggleState(true, shutup); break;
    case NoteStealPolicyStealOldest: steal1.setToggleState(true, shutup); break;
    case NoteStealPolicyStealLowestRMS: steal2.setToggleState(true, shutup); break;
    case NoteStealPolicyStealIfLower: steal3.setToggleState(true, shutup); break;
    case NoteStealPolicyStealIfHigher: steal4.setToggleState(true, shutup); break;
    case NoteStealPolicyStealYoungest: steal5.setToggleState(true, shutup); break;
    default: PP_NYI; break;
    }

    switch (settings.legatoMode) {
    case LegatoModeRetrigger: legato0.setToggleState(true, shutup); break;
    case LegatoModeUpdateVelocity: legato1.setToggleState(true, shutup); break;
    case LegatoModeFreezeVelocity: legato2.setToggleState(true, shutup); break;
    case LegatoModeReleaseVelocity: legato3.setToggleState(true, shutup); break;
    default: PP_NYI; break;
    }

    switch (settings.noteReactivationPolicy) {
    case NoteReactivationPolicyDoNotReactivate: reactivate0.setToggleState(true, shutup); break;
    case NoteReactivationPolicyLast: reactivate1.setToggleState(true, shutup); break;
    case NoteReactivationPolicyHighest: reactivate2.setToggleState(true, shutup); break;
    case NoteReactivationPolicyLowest: reactivate3.setToggleState(true, shutup); break;
    case NoteReactivationPolicyFirst: reactivate4.setToggleState(true, shutup); break;
    default: PP_NYI; break;
    }

    switch (settings.noteActivationPolicy) {
    case NoteActivationPolicyOnlySilent: activate0.setToggleState(true, shutup); break;
    case NoteActivationPolicyOldest: activate1.setToggleState(true, shutup); break;
    default: PP_NYI; break;
    }

}

SettingsEditor::SettingsEditor(SubValue<PresetSettings>& subSettings_)
    : subSettings(subSettings_)
{
    addAndMakeVisible(&grid, false);
    subSettingsHandle = subSettings.subscribe([this](const PhasePhckr::PresetSettings& s) {
        settings = s;
        applySettings();
    });

    // polyphony
    polyGrid.setText("polyphony");
    grid.addComponent(&polyGrid);

    polyphony1.addListener(this);
    polyphony2.addListener(this);
    polyphony4.addListener(this);
    polyphony8.addListener(this);
    polyphony16.addListener(this);
    polyphony32.addListener(this);
    polyphony64.addListener(this);

    polyGrid.addComponent(&polyphony1);
    polyGrid.addComponent(&polyphony2);
    polyGrid.addComponent(&polyphony4);
    polyGrid.addComponent(&polyphony8);
    polyGrid.addComponent(&polyphony16);
    polyGrid.addComponent(&polyphony32);
    polyGrid.addComponent(&polyphony64);

    polyphony1.setButtonText("1");
    polyphony2.setButtonText("2");
    polyphony4.setButtonText("4");
    polyphony8.setButtonText("8");
    polyphony16.setButtonText("16");
    polyphony32.setButtonText("32");
    polyphony64.setButtonText("64");

    polyphony1.setRadioGroupId(1);
    polyphony2.setRadioGroupId(1);
    polyphony4.setRadioGroupId(1);
    polyphony8.setRadioGroupId(1);
    polyphony16.setRadioGroupId(1);
    polyphony32.setRadioGroupId(1);
    polyphony64.setRadioGroupId(1);

    polyphony1.setClickingTogglesState(true);
    polyphony2.setClickingTogglesState(true);
    polyphony4.setClickingTogglesState(true);
    polyphony8.setClickingTogglesState(true);
    polyphony16.setClickingTogglesState(true);
    polyphony32.setClickingTogglesState(true);
    polyphony64.setClickingTogglesState(true);

    // steal policy
    stealGrid.setText("note steal policy");
    grid.addComponent(&stealGrid);

    steal0.addListener(this);
    steal1.addListener(this);
    steal2.addListener(this);
    steal3.addListener(this);
    steal4.addListener(this);
    steal5.addListener(this);

    stealGrid.addComponent(&steal0);
    stealGrid.addComponent(&steal1);
    stealGrid.addComponent(&steal2);
    stealGrid.addComponent(&steal3);
    stealGrid.addComponent(&steal4);
    stealGrid.addComponent(&steal5);

    steal0.setButtonText("do not steal");
    steal1.setButtonText("oldest");
    steal2.setButtonText("lowest rms");
    steal3.setButtonText("if lower");
    steal4.setButtonText("if higher");
    steal5.setButtonText("youngest");

    steal0.setRadioGroupId(2);
    steal1.setRadioGroupId(2);
    steal2.setRadioGroupId(2);
    steal3.setRadioGroupId(2);
    steal4.setRadioGroupId(2);
    steal5.setRadioGroupId(2);

    steal0.setClickingTogglesState(true);
    steal1.setClickingTogglesState(true);
    steal2.setClickingTogglesState(true);
    steal3.setClickingTogglesState(true);
    steal4.setClickingTogglesState(true);
    steal5.setClickingTogglesState(true);

    // legato mode
    legatoGrid.setText("legato mode");
    grid.addComponent(&legatoGrid);

    legato0.addListener(this);
    legato1.addListener(this);
    legato2.addListener(this);
    legato3.addListener(this);

    legatoGrid.addComponent(&legato0);
    legatoGrid.addComponent(&legato1);
    legatoGrid.addComponent(&legato2);
    legatoGrid.addComponent(&legato3);

    legato0.setButtonText("retrigger");
    legato1.setButtonText("legato, update velocity");
    legato2.setButtonText("legato, freeze velocity");
    legato3.setButtonText("legato, release velocity");

    legato0.setRadioGroupId(4);
    legato1.setRadioGroupId(4);
    legato2.setRadioGroupId(4);
    legato3.setRadioGroupId(4);

    legato0.setClickingTogglesState(true);
    legato1.setClickingTogglesState(true);
    legato2.setClickingTogglesState(true);
    legato3.setClickingTogglesState(true);

    // reactivation policy
    reactivateGrid.setText("note reactivation policy");
    grid.addComponent(&reactivateGrid);

    reactivate0.addListener(this);
    reactivate1.addListener(this);
    reactivate2.addListener(this);
    reactivate3.addListener(this);
    reactivate4.addListener(this);

    reactivateGrid.addComponent(&reactivate0);
    reactivateGrid.addComponent(&reactivate1);
    reactivateGrid.addComponent(&reactivate2);
    reactivateGrid.addComponent(&reactivate3);
    reactivateGrid.addComponent(&reactivate4);

    reactivate0.setButtonText("do not reactivate");
    reactivate1.setButtonText("reactivate last");
    reactivate2.setButtonText("reactivate highest");
    reactivate3.setButtonText("reactivate lowest");
    reactivate4.setButtonText("reactivate first");

    reactivate0.setRadioGroupId(3);
    reactivate1.setRadioGroupId(3);
    reactivate2.setRadioGroupId(3);
    reactivate3.setRadioGroupId(3);
    reactivate4.setRadioGroupId(3);

    reactivate0.setClickingTogglesState(true);
    reactivate1.setClickingTogglesState(true);
    reactivate2.setClickingTogglesState(true);
    reactivate3.setClickingTogglesState(true);
    reactivate4.setClickingTogglesState(true);

    // activation policy
    activateGrid.setText("note activation policy");
    grid.addComponent(&activateGrid);

    activate0.addListener(this);
    activate1.addListener(this);

    activateGrid.addComponent(&activate0);
    activateGrid.addComponent(&activate1);

    activate0.setButtonText("only inactive silent");
    activate1.setButtonText("prefer inactive silent, then pick oldest inactive non-silent");

    activate0.setRadioGroupId(5);
    activate1.setRadioGroupId(5);

    activate0.setClickingTogglesState(true);
    activate1.setClickingTogglesState(true);

    applySettings();

    resized();
}

void SettingsEditor::buttonClicked(Button* b) {
    if (b->getToggleState() == false) return;
    else if (b == &polyphony1) settings.polyphony = 1;
    else if (b == &polyphony2) settings.polyphony = 2;
    else if (b == &polyphony4) settings.polyphony = 4;
    else if (b == &polyphony8) settings.polyphony = 8;
    else if (b == &polyphony16) settings.polyphony = 16;
    else if (b == &polyphony32) settings.polyphony = 32;
    else if (b == &polyphony64) settings.polyphony = 64;

    else if (b == &steal0) settings.noteStealPolicy = NoteStealPolicyDoNotSteal;
    else if (b == &steal1) settings.noteStealPolicy = NoteStealPolicyStealOldest;
    else if (b == &steal2) settings.noteStealPolicy = NoteStealPolicyStealLowestRMS;
    else if (b == &steal3) settings.noteStealPolicy = NoteStealPolicyStealIfLower;
    else if (b == &steal4) settings.noteStealPolicy = NoteStealPolicyStealIfHigher;
    else if (b == &steal5) settings.noteStealPolicy = NoteStealPolicyStealYoungest;

    else if (b == &legato0) settings.legatoMode = LegatoModeRetrigger;
    else if (b == &legato1) settings.legatoMode = LegatoModeUpdateVelocity;
    else if (b == &legato2) settings.legatoMode = LegatoModeFreezeVelocity;
    else if (b == &legato3) settings.legatoMode = LegatoModeReleaseVelocity;

    else if (b == &reactivate0) settings.noteReactivationPolicy = NoteReactivationPolicyDoNotReactivate;
    else if (b == &reactivate1) settings.noteReactivationPolicy = NoteReactivationPolicyLast;
    else if (b == &reactivate2) settings.noteReactivationPolicy = NoteReactivationPolicyHighest;
    else if (b == &reactivate3) settings.noteReactivationPolicy = NoteReactivationPolicyLowest;
    else if (b == &reactivate4) settings.noteReactivationPolicy = NoteReactivationPolicyFirst;

    else if (b == &activate0) settings.noteActivationPolicy = NoteActivationPolicyOnlySilent;
    else if (b == &activate1) settings.noteActivationPolicy = NoteActivationPolicyOldest;

    else PP_NYI;

    subSettings.set(subSettingsHandle, settings);
}