#pragma once

#include "JuceLibraryCode/JuceHeader.h"

class PhasePhckrLookAndFeel : public LookAndFeel_V3
{
public:
    PhasePhckrLookAndFeel()
    {
        setColour (Slider::rotarySliderFillColourId, Colours::red);
    }
};

Colour g_tabColor = Colours::black;