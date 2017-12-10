#include "JuceHeader.h"
#include "PhasePhckrPluginProcessorFX.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhasePhckrProcessorFX();
}