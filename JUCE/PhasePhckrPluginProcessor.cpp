/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrPluginEditor.h"

//==============================================================================
PhasePhckrAudioProcessor::PhasePhckrAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

PhasePhckrAudioProcessor::~PhasePhckrAudioProcessor()
{
}

//==============================================================================
const String PhasePhckrAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhasePhckrAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PhasePhckrAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double PhasePhckrAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhasePhckrAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PhasePhckrAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PhasePhckrAudioProcessor::setCurrentProgram (int index)
{
}

const String PhasePhckrAudioProcessor::getProgramName (int index)
{
    return String();
}

void PhasePhckrAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void PhasePhckrAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void PhasePhckrAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PhasePhckrAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // We only support stereo output ...
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::stereo()) return false;

    // This checks if the input layout matches the output layoutsustainHeight
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PhasePhckrAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numMidiMessages = midiMessages.getNumEvents();
    const int blockSize = buffer.getNumSamples();
    float sampleRate = getSampleRate();

    for (int i = 0; i < numOutputChannels; ++i) {
        buffer.clear(i, 0, blockSize);
    }

    // TODO, run in some internal chunk size. make sure that midi messages are queued in time as they should.

    MidiBuffer::Iterator midiIt(midiMessages);
    int evtPos = 0;
    MidiMessage msg;
    while (midiIt.getNextEvent(msg, evtPos)){
        int ch = msg.getChannel();
        if(msg.isNoteOnOrOff()){
            synth.voiceBus.handleNoteOnOff(
                ch, 
                msg.getNoteNumber(),
                msg.getFloatVelocity(), 
                msg.isNoteOn(true)
            );
        }
        else if(msg.isPitchWheel()){
            synth.voiceBus.handleX(
                ch,
                2.f*( (float)msg.getPitchWheelValue() / (float)(0x3fff) - 0.5f)
            );
        }
        else if(msg.isAftertouch()){
            synth.voiceBus.handleNoteZ(
                ch,
                msg.getNoteNumber(),
                (float)msg.getAfterTouchValue() / 127.f
            );
        }
        else if(msg.isChannelPressure()){
            synth.voiceBus.handleZ(
                ch,
                (float)msg.getChannelPressureValue() / 127.f
            );
        }
        else if(msg.isController()){
            int cc = msg.getControllerNumber();
            float val = (float)msg.getControllerValue() / 127.f;
            // TODO, LSB for 1,2,11 (33,34,43) in a standard compliant way
            switch (cc) {
                case 74:
                    synth.voiceBus.handleY(ch, val);
                    break;
                case 1:
                    synth.voiceBus.handleModWheel(val);
                    break;
                case 2:
                    synth.voiceBus.handleBreath(val);
                    break;
                case 11:
                    synth.voiceBus.handleExpression(val);
                    break;
                default:
                    break;
            }
        }
    }
    midiMessages.clear();

    // mono (clono) for now until we have a concept of a mixer
    synth.update(buffer.getWritePointer(0), blockSize, sampleRate);
    for (int channel = 1; channel < numOutputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        buffer.copyFrom(channel, 0, buffer.getReadPointer(0), blockSize);
    }
}

//==============================================================================
bool PhasePhckrAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* PhasePhckrAudioProcessor::createEditor()
{
    return new PhasePhckrAudioProcessorEditor (*this);
}

//==============================================================================
void PhasePhckrAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PhasePhckrAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhasePhckrAudioProcessor();
}

const PhasePhckr::Synth& PhasePhckrAudioProcessor::getSynth() const {
    return synth;
}