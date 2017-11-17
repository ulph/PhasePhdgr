#include <phasephckr_json.hpp>
#include <xmmintrin.h>

#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrPluginEditor.h"
#include "FileIO.hpp"

using namespace PhasePhckrFileStuff;
using namespace std;

void PhasePhckrAudioProcessor::updateComponentRegister(const DirectoryContentsList* d)
{
    for(int i=0; i<d->getNumFiles(); i++) {
        const File& f = d->getFile(i);
        if(!f.existsAsFile()) continue; // TODO, recurse into subdirs
        String p = f.getRelativePathFrom(componentsDir);
        string n = string(&componentMarker)+p.dropLastCharacters(5).toUpperCase().toStdString(); // remove .json
        string s = f.loadFileAsString().toStdString();
        try {
            json j = json::parse(s.c_str());
            ComponentDescriptor cd = j;
            componentRegister.registerComponent(n, cd);
            subComponentRegister.set(componentRegisterHandle, componentRegister);
        } catch (const std::exception& e) {
            (void)e;
            continue;
            assert(0);
        }
    }
}

PhasePhckrAudioProcessor::PhasePhckrAudioProcessor()
     : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo(), true))
     , fileWatchThread("processorFileWatchThread")
     , componentDirectoryWatcher(getFilter(), fileWatchThread)
     , componentFilesListener(StupidFileListCallBack([this](const DirectoryContentsList* d){updateComponentRegister(d);}))
{
    activeVoiceHandle = subVoiceChain.subscribe([this](const PhasePhckr::PatchDescriptor& v){setVoiceChain(v);});
    activeEffectHandle = subEffectChain.subscribe([this](const PhasePhckr::PatchDescriptor& e){setEffectChain(e);});
    componentRegisterHandle = subComponentRegister.subscribe([this](const PhasePhckr::ComponentRegister& cr){ /**/ });

    createInitialUserLibrary(componentRegister); // TODO, only do this on FIRST start

    fileWatchThread.startThread();
    componentDirectoryWatcher.addChangeListener(&componentFilesListener);
    componentDirectoryWatcher.setDirectory(componentsDir, true, true);

    // parameter mumbo
    parameters.initialize(this);

    // create the synth and push down the initial chains
    synth = new PhasePhckr::Synth();

    PresetDescriptor initialPreset;
    initialPreset.voice = getExampleVoiceChain();
    initialPreset.effect = getExampleEffectChain();

    setPreset(initialPreset);

}

PhasePhckrAudioProcessor::~PhasePhckrAudioProcessor()
{
    subVoiceChain.unsubscribe(activeVoiceHandle);
    subEffectChain.unsubscribe(activeEffectHandle);
    subComponentRegister.unsubscribe(componentRegisterHandle);
    delete synth;
}

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

    // handle MIDI messages
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

    // handle the parameter values
    parameters.visitHandleParameterValues(synth);

    // handle HOST properties
    auto playHead = getPlayHead();
    if(playHead){
        AudioPlayHead::CurrentPositionInfo info;
        playHead->getCurrentPosition(info);
        synth->handleTimeSignature(info.timeSigNumerator, info.timeSigDenominator);
        synth->handleBPM((float)info.bpm);
        synth->handlePosition((float)info.ppqPosition);
        synth->handleTime((float)info.timeInSeconds);
    }

    synth->update(buffer.getWritePointer(0), buffer.getWritePointer(1), blockSize, sampleRate);

    synthUpdateLock.clear(std::memory_order_release);

#if FORCE_FTZ_DAZ
    _mm_setcsr( oldMXCSR );
#endif
}

bool PhasePhckrAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* PhasePhckrAudioProcessor::createEditor()
{
    return new PhasePhckrAudioProcessorEditor (*this);
}

void PhasePhckrAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    json j = getPreset();
    string ss = j.dump(2); // json lib bugged with long rows
    const char* s = ss.c_str(); 
    size_t n = (strlen(s)+1) / sizeof(char);
    destData.fillWith(0);
    destData.insert((const void*)s, n, 0);
    assert(destData.getSize() == n);
}

void PhasePhckrAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    string ss((const char*)data);
    PresetDescriptor preset;
    try {
        preset = json::parse(ss.c_str());
    }
    catch (const nlohmann::detail::exception& e) {
        auto msg = e.what();
        // sod off
        return;
    }
    setPreset(preset);
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhasePhckrAudioProcessor();
}

const PhasePhckr::Synth* PhasePhckrAudioProcessor::getSynth() const {
    return synth;
}

void PhasePhckrAudioProcessor::broadcastPatch() {
    // editor should call this once after construction
    subComponentRegister.set(componentRegisterHandle, componentRegister);
    subVoiceChain.set(activeVoiceHandle, voiceChain);
    subEffectChain.set(activeEffectHandle, effectChain);
}

PatchDescriptor PhasePhckrAudioProcessor::getPatch(SynthGraphType type, bool extractParameters) {
    PatchDescriptor patch;

    while (synthUpdateLock.test_and_set(std::memory_order_acquire));
    if (type == VOICE) patch = voiceChain;
    else if (type == EFFECT) patch = effectChain;
    synthUpdateLock.clear(std::memory_order_release);

    if (extractParameters) patch.parameters = getParameters(type);

    return patch;
}

void PhasePhckrAudioProcessor::setPatch(SynthGraphType type, const PatchDescriptor& patch) {
    if (type == VOICE) {
        setVoiceChain(patch);
        subVoiceChain.set(activeVoiceHandle, patch);
    }
    else if (type == EFFECT) {
        setEffectChain(patch);
        subEffectChain.set(activeEffectHandle, patch);
    }
}

PresetDescriptor PhasePhckrAudioProcessor::getPreset() {
    PresetDescriptor preset;

    preset.voice = getPatch(VOICE);
    preset.effect = getPatch(EFFECT);
    preset.parameters = parameters.serialize();

    return preset;
}

vector<PresetParameterDescriptor> PhasePhckrAudioProcessor::getPresetParameters() {
    return parameters.serialize();
}

vector<PatchParameterDescriptor> PhasePhckrAudioProcessor::getParameters(SynthGraphType type) {
    vector<PresetParameterDescriptor> presetParams = parameters.serialize();
    vector<PatchParameterDescriptor> params;

    for (const auto& ppd : presetParams) {
        if (ppd.p.type == type) {
            auto pd = ppd.p;
            params.emplace_back(pd);
        }
    }

    return params;
}

void PhasePhckrAudioProcessor::setPreset(const PresetDescriptor& preset) {
    setPatch(VOICE, preset.voice);
    setPatch(EFFECT, preset.effect);
    parameters.deserialize(preset.parameters);
}

void PhasePhckrAudioProcessor::setVoiceChain(const PhasePhckr::PatchDescriptor &p) {
    while (synthUpdateLock.test_and_set(std::memory_order_acquire));

    voiceChain = p;
    auto pv = synth->setVoiceChain(voiceChain, componentRegister);
    parameters.setParametersHandleMap(VOICE, pv);

    synthUpdateLock.clear(std::memory_order_release);

}

void PhasePhckrAudioProcessor::setEffectChain(const PhasePhckr::PatchDescriptor &p) {
    while (synthUpdateLock.test_and_set(std::memory_order_acquire));

    effectChain = p;
    auto pv = synth->setEffectChain(effectChain, componentRegister);
    parameters.setParametersHandleMap(EFFECT, pv);

    synthUpdateLock.clear(std::memory_order_release);
}
