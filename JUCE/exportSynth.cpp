#include "JuceHeader.h"
#include "PhasePhckrPluginProcessor.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhasePhckrProcessor();
}