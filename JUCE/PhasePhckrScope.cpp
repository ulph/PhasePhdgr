#include "JuceLibraryCode/JuceHeader.h"
#include "PhasePhckrScope.h"

void PhasePhckrScope::paint (Graphics& g)
{
    float size_y = (float)this->getHeight();
    float size_x = (float)this->getWidth();
    float yScale = size_y / (2.0f*1.0f + 0.75f);

    g.setColour(Colours::brown);
    g.drawLine(0.0f, (size_y*0.5f), size_x, (size_y*0.5f), 0.5f);
    g.drawLine(0.0f, (size_y*0.5f + yScale), size_x, (size_y*0.5f + yScale), 0.5f);
    g.drawLine(0.0f, (size_y*0.5f - yScale), size_x, (size_y*0.5f - yScale), 0.5f);

    int sourceSize = 0;
    const float * sourceBuffer = source.getBuffer(&sourceSize);

    g.setColour(Colours::yellow);
    if (sourceSize > 1 && size_x > 1) {
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


void PhasePhckrXYScope::paint (Graphics& g)
{
    float size_y = (float)this->getHeight();
    float size_x = (float)this->getWidth();
    int sourceSizeL = 0;
    const float * sourceBufferL = sourceL.getBuffer(&sourceSizeL);
    int sourceSizeR = 0;
    const float * sourceBufferR = sourceR.getBuffer(&sourceSizeR);

    if(sourceSizeL != sourceSizeR || sourceSizeL < 0){ repaint(); return; }

    float blitScale = 0.5;
    float lineScale = 0.75;

    g.setColour(Colours::brown);
    for (int i = 0; i < sourceSizeL-1; ++i) {
        g.drawLine(
            0.5*size_x*(lineScale*sourceBufferL[i]+0.5f),
            size_y*(lineScale*sourceBufferR[i]+0.5f),
            0.5*size_x*(lineScale*sourceBufferL[i+1]+0.5f),
            size_y*(lineScale*sourceBufferR[i+1]+0.5f),
            0.25f
        );
    }

    g.setColour(Colours::yellow);
    for (int i = 0; i < sourceSizeL; ++i) {
        g.setPixel(
            0.5*size_x*(blitScale*sourceBufferL[i]+0.5),
            size_y*(blitScale*sourceBufferR[i]+0.5)
        );
    }

    repaint();
}

void PhasePhckrXYScope::resized()
{
    repaint();
}
