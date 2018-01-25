#include "JuceHeader.h"
#include "ScopeView.h"

void ScopeView::paint (Graphics& g)
{
    const float y_clip = 2.f;

    float size_y = (float)this->getHeight();
    float size_x = (float)this->getWidth();
    float yScale = size_y / (2.0f*1.0f + 0.75f);

    int sourceSize = 0;
    const float * sourceBuffer = source.getBuffer(&sourceSize);

    g.setColour(Colour(0xff880000));
    g.fillRect(0.0f, (size_y*0.5f         ), size_x, 1.0f);
    g.fillRect(0.0f, (size_y*0.5f + yScale), size_x, 1.0f);
    g.fillRect(0.0f, (size_y*0.5f - yScale), size_x, 1.0f);

    float clipLevel = 0.0f;

    if (sourceSize > 1 && size_x > 1) {
        float xScale = size_x / (float)(sourceSize - 1);
        g.setColour(Colour(0xffffff00));
        for (int i = 0; i < (sourceSize - 1); ++i) {
            float y0 = sourceBuffer[i];
            float y1 = sourceBuffer[i + 1];

            if (fabsf(y1) > 1.0f || fabsf(y0) > 1.0f) {
                clipLevel = fmaxf(clipLevel, fmaxf(fabsf(y1), fabsf(y0)));
            }

            y0 = y0 > y_clip ? y_clip : y0;
            y0 = y0 < -y_clip ? -y_clip : y0;
            y1 = y1 > y_clip ? y_clip : y1;
            y1 = y1 < -y_clip ? -y_clip : y1;

            g.drawLine(
                i*xScale,
                size_y*0.5f - yScale*sourceBuffer[i],
                (i + 1)*xScale,
                size_y*0.5f - yScale*sourceBuffer[i + 1],
                0.5f
            );
        }
    }
    
    if (clipLevel) {
        float alpha = clipLevel * 0.03f;
        g.setColour(Colours::red.withAlpha(alpha));
        g.fillAll();
    }
}

void ScopeView::resized()
{
    repaint();
}


void XYScopeView::paint (Graphics& g)
{
    float size_y = (float)this->getHeight();
    float size_x = (float)this->getWidth();
    int sourceSizeL = 0;
    const float * sourceBufferL = sourceL.getBuffer(&sourceSizeL);
    int sourceSizeR = 0;
    const float * sourceBufferR = sourceR.getBuffer(&sourceSizeR);

    if(sourceSizeL != sourceSizeR || sourceSizeL < 0){ repaint(); return; }

    float blitScale = 0.5f;
    const float clip = 2.f;

    float gridScale = 1.0f / (float)sourceSizeL;

    g.setColour(Colour(0x88ffff00));
    for (int i = 0; i < sourceSizeL; ++i) {
        float x = (sourceBufferR[i] - sourceBufferL[i]) / sqrtf(2.0f);
        float y = (sourceBufferL[i] + sourceBufferR[i]) / sqrtf(2.0f);

        y = y > clip ? clip : y;
        y = y < -clip ? -clip : y;
        x = x > clip ? clip : x;
        x = x < -clip ? -clip : x;

		g.fillRect(
			size_x*( blitScale*x + 0.5f),
			size_y*(-blitScale*y + 0.5f),
			2.f,
			2.f
        );
    }
}

void XYScopeView::resized()
{
    repaint();
}
