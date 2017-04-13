#include <string.h>

#include "delay.hpp"


Delay::Delay() 
    : readPosition(0)
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("time", 0.5f));
    inputs.push_back(Pad("gain", 0.5f));
    outputs.push_back(Pad("out"));
    memset(buffer, 0, sizeof(buffer));
}

void Delay::process(uint32_t fs) {
    float t = inputs[1].value;
    float g = inputs[2].value;

    // limit time ranges
    t = t > 5.0f ? 5.0f : t < 0.0f ? 0.0f : t;
    // buffersize is calculated for 5s @ 96kHz ... so 192kHz etc may not work as expected

    int bufferSize = sizeof(buffer) / sizeof(float);

    int writePosition = (readPosition + (int)(t*fs)) % bufferSize;
    buffer[writePosition] = g*inputs[0].value;
    outputs[0].value = buffer[readPosition];
    readPosition++;
    readPosition %= bufferSize;

}
