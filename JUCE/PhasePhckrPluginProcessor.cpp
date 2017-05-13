/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrPluginEditor.h"
#include "design_json.hpp"
#include "DirectoryWatcher.hpp"

#include <xmmintrin.h>

using namespace PhasePhckrFileStuff;

PhasePhckrAudioProcessor::PhasePhckrAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                       )
{

    activeVoiceHandle = activeVoice.subscribe([this](const PhasePhckr::PatchDescriptor& v){setVoiceChain(v);});
    activeEffectHandle = activeEffect.subscribe([this](const PhasePhckr::PatchDescriptor& e){setEffectChain(e);});
    componentRegisterHandle = subComponentRegister.subscribe([this](const PhasePhckr::ComponentRegister& cr){ /**/ });

    createDirIfNeeded(rootDir);
    createDirIfNeeded(effectsDir);
    createDirIfNeeded(voicesDir);
    createDirIfNeeded(componentsDir);
    createDirIfNeeded(patchesDir);

    // load init patches and dump to disk
    File initVoice = voicesDir.getFullPathName() + File::separator + "_init.json";
    File initEffect = effectsDir.getFullPathName() + File::separator + "_init.json"; 

    voiceChain = PhasePhckr::getExampleVoiceChain();
    initVoice.replaceWithText(json(voiceChain).dump(2));

    effectChain = PhasePhckr::getExampleFxChain();
    initEffect.replaceWithText(json(effectChain).dump(2));

    // dump all factory components to disk
    for (const auto &kv : componentRegister.all()) {
        const auto &type = kv.first;
        const auto &body = kv.second;
        File cmp = componentsDir.getFullPathName() + File::separator + type.substr(1) + ".json";
        cmp.replaceWithText(json(body).dump(2));
    }

    // create the synth and push down the initial chains
    synth = new PhasePhckr::Synth();

    applyVoiceChain();
    applyEffectChain();

    for (int i = 0; i < 8*32; i++) {
        auto knb_ptr = new PhasePhckrParameter(i);
        floatParameters.push_back(knb_ptr);
        addParameter(knb_ptr);
    }
}

PhasePhckrAudioProcessor::~PhasePhckrAudioProcessor()
{
    activeVoice.unsubscribe(activeVoiceHandle);
    activeEffect.unsubscribe(activeEffectHandle);
    subComponentRegister.unsubscribe(componentRegisterHandle);
    delete synth;
}

void PhasePhckrAudioProcessor::applyVoiceChain() {
    while (synthUpdateLock.test_and_set(std::memory_order_acquire));
    synth->setVoiceChain(voiceChain, componentRegister);
    synthUpdateLock.clear(std::memory_order_release);
}

void PhasePhckrAudioProcessor::applyEffectChain() {
    while (synthUpdateLock.test_and_set(std::memory_order_acquire));
    synth->setFxChain(effectChain, componentRegister);
    synthUpdateLock.clear(std::memory_order_release);
}

//==============================================================================
const String PhasePhckrAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhasePhckrAudioProcessor::acceptsMidi() const
{
    return true;
}

bool PhasePhckrAudioProcessor::producesMidi() const
{
    return false;
}

double PhasePhckrAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhasePhckrAudioProcessor::getNumPrograms()
{
    return 1;
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
}

void PhasePhckrAudioProcessor::releaseResources()
{
}

bool PhasePhckrAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::stereo()) return false;
    return true;
}

void PhasePhckrAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
#if FORCE_FTZ_DAZ
    int oldMXCSR = _mm_getcsr(); /*read the old MXCSR setting */ \
    int newMXCSR = oldMXCSR | 0x8040; /* set DAZ and FZ bits */ \
    _mm_setcsr( newMXCSR); /*write the new MXCSR setting to the MXCSR */
#endif

    while (synthUpdateLock.test_and_set(std::memory_order_acquire));

    const int numOutputChannels = getTotalNumOutputChannels();
//    const int numMidiMessages = midiMessages.getNumEvents();
    const int blockSize = buffer.getNumSamples();
    float sampleRate = (float)getSampleRate();

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
            synth->handleNoteOnOff(
                ch, 
                msg.getNoteNumber(),
                msg.getFloatVelocity(), 
                msg.isNoteOn(true)
            );
        }
        else if(msg.isPitchWheel()){
            synth->handleX(
                ch,
                2.f*( (float)msg.getPitchWheelValue() / (float)(0x3fff) - 0.5f)
            );
        }
        else if(msg.isAftertouch()){
            synth->handleNoteZ(
                ch,
                msg.getNoteNumber(),
                (float)msg.getAfterTouchValue() / 127.f
            );
        }
        else if(msg.isChannelPressure()){
            synth->handleZ(
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
                    synth->handleY(ch, val);
                    break;
                case 1:
                    synth->handleModWheel(val);
                    break;
                case 2:
                    synth->handleBreath(val);
                    break;
                case 11:
                    synth->handleExpression(val);
                    break;
                default:
                    break;
            }
        }
    }
    midiMessages.clear();

    synth->update(buffer.getWritePointer(0), buffer.getWritePointer(1), blockSize, sampleRate);

    synthUpdateLock.clear(std::memory_order_release);

#if FORCE_FTZ_DAZ
    _mm_setcsr( oldMXCSR );
#endif
}

//==============================================================================
bool PhasePhckrAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* PhasePhckrAudioProcessor::createEditor()
{
    return new PhasePhckrAudioProcessorEditor (*this);
}

//==============================================================================
void PhasePhckrAudioProcessor::getStateInformation (MemoryBlock& destData)
{
}

void PhasePhckrAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhasePhckrAudioProcessor();
}

const PhasePhckr::Synth* PhasePhckrAudioProcessor::getSynth() const {
    return synth;
}
