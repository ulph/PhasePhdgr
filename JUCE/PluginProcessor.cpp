#include <phasephckr_json.hpp>
#include <xmmintrin.h>

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FileIO.hpp"

#include "PluginCommon.h"

using namespace PhasePhckrFileStuff;
using namespace std;

PhasePhckrProcessor::PhasePhckrProcessor()    
    : AudioProcessor(BusesProperties().withOutput("Output", AudioChannelSet::stereo(), true).withInput("Input", AudioChannelSet::disabled(), true))
    , componentLoader(subComponentRegister)
{

    midiMessageQueue.reserve(128); // some nice large-ish number

    activeSettingsHandle = subSettings.subscribe([this](const PhasePhckr::PresetSettings& s){
        setSettings(s);
    });
    activeVoiceHandle = subVoiceChain.subscribe([this](const PhasePhckr::PatchDescriptor& v){
        setVoiceChain(v);
    });
    activeEffectHandle = subEffectChain.subscribe([this](const PhasePhckr::PatchDescriptor& e){
        setEffectChain(e);
    });
    componentRegisterHandle = subComponentRegister.subscribe([this](const PhasePhckr::ComponentRegister& cr){ 
        setComponentRegister(cr);
        setVoiceChain(voiceChain);
        setEffectChain(effectChain);
    });

    createInitialUserLibrary(componentRegister);

    parameters.initialize(this);

    synth = new PhasePhckr::Synth();
    componentLoader.rescan();

    PresetDescriptor initialPreset;
    initialPreset.voice = getExampleVoiceChain();
    initialPreset.effect = getExampleEffectChain();

    setPreset(initialPreset);
}

PhasePhckrProcessor::~PhasePhckrProcessor()
{
    subVoiceChain.unsubscribe(activeVoiceHandle);
    subEffectChain.unsubscribe(activeEffectHandle);
    subComponentRegister.unsubscribe(componentRegisterHandle);
    delete synth;
}

const String PhasePhckrProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhasePhckrProcessor::acceptsMidi() const
{
    return true;
}

bool PhasePhckrProcessor::producesMidi() const
{
    return false;
}

double PhasePhckrProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhasePhckrProcessor::getNumPrograms()
{
    return 1;
}

int PhasePhckrProcessor::getCurrentProgram()
{
    return 0;
}

void PhasePhckrProcessor::setCurrentProgram (int index)
{
}

const String PhasePhckrProcessor::getProgramName (int index)
{
    return String();
}

void PhasePhckrProcessor::changeProgramName (int index, const String& newName)
{
}

void PhasePhckrProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void PhasePhckrProcessor::releaseResources()
{
}

bool PhasePhckrProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::stereo()) return false;
    return true;
}

void PhasePhckrProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    auto dn = ScopedNoDenormals();

    const int numOutputChannels = getTotalNumOutputChannels();
    for (int i = 0; i < numOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    auto l = synthUpdateLock.make_scoped_lock();

    // handle MIDI messages
    MidiBuffer::Iterator midiIt(midiMessages);
    int evtPos = 0;
    MidiMessage msg;
    while (midiIt.getNextEvent(msg, evtPos)){
        int ch = msg.getChannel();
        if(msg.isNoteOnOrOff()){
            midiMessageQueue.emplace_back(
                msg.isNoteOn(true) ? PPMidiMessage::Type::On : PPMidiMessage::Type::Off,
                evtPos,
                ch,
                msg.getNoteNumber(),
                msg.getFloatVelocity()
            );
        }
        else if(msg.isPitchWheel()){
            midiMessageQueue.emplace_back(
                PPMidiMessage::Type::X,
                ch,
                2.f*((float)msg.getPitchWheelValue() / (float)(0x3fff) - 0.5f)
            );
        }
        else if(msg.isAftertouch()){
            midiMessageQueue.emplace_back(
                PPMidiMessage::Type::NoteZ,
                ch,
                msg.getNoteNumber(),
                (float)msg.getAfterTouchValue() / 127.f
            );
        }
        else if(msg.isChannelPressure()){
            midiMessageQueue.emplace_back(
                PPMidiMessage::Type::Z,
                ch,
                (float)msg.getChannelPressureValue() / 127.f
            );
        }
        else if(msg.isController()){
            int cc = msg.getControllerNumber();
            float val = (float)msg.getControllerValue() / 127.f;
            // TODO, LSB for 1,2,11 (33,34,43) in a standard compliant way
            switch (cc) {
                case 64:
                    midiMessageQueue.emplace_back(
                        PPMidiMessage::Type::Sustain,
                        ch,
                        val
                    );
                    break;
                case 74:
                    midiMessageQueue.emplace_back(
                        PPMidiMessage::Type::Y,
                        ch,
                        val
                    );
                    break;
                case 1:
                    midiMessageQueue.emplace_back(
                        PPMidiMessage::Type::ModWheel,
                        ch,
                        val
                    );
                    break;
                case 2:
                    midiMessageQueue.emplace_back(
                        PPMidiMessage::Type::Breath,
                        ch,
                        val
                    );
                    break;
                case 11:
                    midiMessageQueue.emplace_back(
                        PPMidiMessage::Type::Expression,
                        ch,
                        val
                    );
                    break;
                default:
                    break;
            }
        }
    }

    parameters.visitHandleParameterValues(synth);
    bufferingProcessor.process(buffer, midiMessageQueue, (float)getSampleRate(), synth, getPlayHead());

    midiMessages.clear();

}

bool PhasePhckrProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* PhasePhckrProcessor::createEditor()
{
    return new PhasePhckrEditor (*this);
}

void PhasePhckrProcessor::getStateInformation (MemoryBlock& destData)
{
    auto p = getPreset();
    storeState(p, destData);
}

void PhasePhckrProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    PresetDescriptor preset;
    loadState(data, sizeInBytes, preset);
    setPreset(preset);
}

const PhasePhckr::Synth* PhasePhckrProcessor::getSynth() const {
    return synth;
}

void PhasePhckrProcessor::broadcastPatch() {
    // editor should call this once after construction
    subComponentRegister.set(componentRegisterHandle, componentRegister);
    subVoiceChain.set(activeVoiceHandle, voiceChain);
    subEffectChain.set(activeEffectHandle, effectChain);
    subSettings.set(activeSettingsHandle, activeSettings);
}

PatchDescriptor PhasePhckrProcessor::getPatch(SynthGraphType type, bool extractParameters) {
    PatchDescriptor patch;

    auto scoped_lock = synthUpdateLock.make_scoped_lock();

    if (type == VOICE) patch = voiceChain;
    else if (type == EFFECT) patch = effectChain;

    if (extractParameters) patch.parameters = getParameters(type);

    patch.cleanUp();

    return patch;
}

void PhasePhckrProcessor::setPatch(SynthGraphType type, const PatchDescriptor& patch) {
    auto patchCopy = patch;
    patchCopy.cleanUp();
    if (type == VOICE) {
        setVoiceChain(patchCopy);
        subVoiceChain.set(activeVoiceHandle, patchCopy);
    }
    else if (type == EFFECT) {
        setEffectChain(patchCopy);
        subEffectChain.set(activeEffectHandle, patchCopy);
    }
}

PresetDescriptor PhasePhckrProcessor::getPreset() {
    PresetDescriptor preset;

    preset.voice = getPatch(VOICE);
    preset.effect = getPatch(EFFECT);
    preset.parameters = parameters.serialize();
    preset.settings = activeSettings;

    return preset;
}

vector<PresetParameterDescriptor> PhasePhckrProcessor::getPresetParameters() {
    return parameters.serialize();
}

vector<PatchParameterDescriptor> PhasePhckrProcessor::getParameters(SynthGraphType type) {
    vector<PresetParameterDescriptor> presetParams = parameters.serialize();
    vector<PatchParameterDescriptor> params;

    for (const auto& ppd : presetParams) {
        if (ppd.type == type) {
            auto pd = ppd.p;
            params.emplace_back(pd);
        }
    }

    return params;
}

void PhasePhckrProcessor::setComponentRegister(const ComponentRegister& cr) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();
    componentRegister = cr;
}

void PhasePhckrProcessor::setPreset(const PresetDescriptor& preset) {
    setPatch(VOICE, preset.voice);
    setPatch(EFFECT, preset.effect);
    setSettings(preset.settings);
    subSettings.set(activeSettingsHandle, activeSettings);
    parameters.deserialize(preset.parameters);
}

void PhasePhckrProcessor::setSettings(const PhasePhckr::PresetSettings &s) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();

    synth->applySettings(s);

    activeSettings = s;

    // hack, as updateHostDisplay() doesn't work for Reaper
    Parameter* pa = nullptr;
    parameters.accessParameter(0, &pa);
    if (pa != nullptr) pa->setValueNotifyingHost(*pa);

    updateHostDisplay();
}

void PhasePhckrProcessor::setVoiceChain(const PhasePhckr::PatchDescriptor &p) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();

    json j = p;
    auto hash = std::hash<string>{}( j.dump() );
    if (hash == voiceHash) return;
    voiceHash = hash;

    voiceChain = p;
    auto pv = synth->setVoiceChain(voiceChain, componentRegister);
    parameters.setParametersHandleMap(VOICE, pv);

    updateHostDisplay();
}

void PhasePhckrProcessor::setEffectChain(const PhasePhckr::PatchDescriptor &p) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();

    json j = p;
    auto hash = std::hash<string>{}(j.dump());
    if (hash == effectHash) return;
    effectHash = hash;

    effectChain = p;
    auto pv = synth->setEffectChain(effectChain, componentRegister);
    parameters.setParametersHandleMap(EFFECT, pv);

    updateHostDisplay();
}

void PhasePhckrProcessor::updateLayout(SynthGraphType type, const string &component, const map<string, ModulePosition> &layout) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();

    auto& p = type == VOICE ? voiceChain : effectChain;
    auto* c = component == "root" ? &p.root : nullptr;
    if (p.components.count(component)) c = &p.components[component];
    if (c == nullptr) return;
    c->layout = layout;

    // hack, as updateHostDisplay() doesn't work for Reaper
    Parameter* pa = nullptr;
    parameters.accessParameter(0, &pa);
    if (pa != nullptr) pa->setValueNotifyingHost(*pa);

    updateHostDisplay();
}
