#include "JuceHeader.h"
#include "PluginProcessorFX.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhasePhckrProcessorFX();
}