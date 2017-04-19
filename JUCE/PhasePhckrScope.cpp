#include "../JuceLibraryCode/JuceHeader.h"
#include "PhasePhckrScope.h"

void PhasePhckrScope::paint (Graphics& g)
{
    float size_y = (float)this->getHeight();
    float size_x = (float)this->getWidth();
    float yScale = size_y / (2.0f*1.0f + 0.25f);

    g.setColour(Colours::brown);
    g.drawHorizontalLine((int)(size_y*0.5f), 0.0f, size_x);
    g.drawHorizontalLine((int)(size_y*0.5f + yScale), 0.0f, size_x);
    g.drawHorizontalLine((int)(size_y*0.5f - yScale), 0.0f, size_x);

    int sourceSize = 0;
    const float * sourceBuffer = source.getBuffer(&sourceSize);

    g.setColour(Colours::yellow);
    if (sourceSize > 1 && size_x > 1) {
        g.setColour(Colours::yellow);
        float xScale = size_x / (float)(sourceSize - 1);
        for (int i = 0; i < (sourceSize - 1); ++i) {
            g.drawLine(
                i*xScale,
                size_y*0.5f + yScale*sourceBuffer[i],
                (i + 1)*xScale,
                size_y*0.5f + yScale*sourceBuffer[i + 1],
                1.f
            );
        }
    }

    repaint();
}

void PhasePhckrScope::resized()
{
    repaint();
}
