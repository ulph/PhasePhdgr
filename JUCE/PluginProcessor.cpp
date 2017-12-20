#include <phasephckr_json.hpp>
#include <xmmintrin.h>

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FileIO.hpp"

#include "PluginCommon.h"

using namespace PhasePhckrFileStuff;
using namespace std;

PhasePhckrProcessor::PhasePhckrProcessor()    
    : AudioProcessor(BusesProperties().withOutput("Output", AudioChannelSet::stereo(), true).withInput("Input", AudioChannelSet::stereo(), true))
    , fileThings(subComponentRegister)
{
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

    createInitialUserLibrary(componentRegister); // TODO, only do this on FIRST start

    // parameter mumbo
    parameters.initialize(this);

    // create the synth and push down the initial chains
    synth = new PhasePhckr::Synth();
    fileThings.rescan();

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
                case 64:
                    synth->handleSustain(ch, val);
                    break;
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

    parameters.visitHandleParameterValues(synth);
    bufferingProcessor.process(buffer, (float)getSampleRate(), synth, getPlayHead());

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
    json j = getPreset();
    string ss = j.dump(2); // json lib bugged with long rows
    const char* s = ss.c_str(); 
    size_t n = (strlen(s)+1) / sizeof(char);
    destData.fillWith(0);
    destData.insert((const void*)s, n, 0);
    assert(destData.getSize() == n);
}

void PhasePhckrProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    string ss((const char*)data);
    PresetDescriptor preset;
    try {
        preset = json::parse(ss.c_str());
    }
    catch (const nlohmann::detail::exception& e) {
        auto msg = e.what();
        cerr << "setStateInformation: " << msg << endl;
        return;
    }
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
    parameters.deserialize(preset.parameters);
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