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
    effect = new PhasePhckr::Effect();
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
    delete effect;
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
                evtPos,
                ch,
                2.f*((float)msg.getPitchWheelValue() / (float)(0x3fff) - 0.5f)
            );
        }
        else if(msg.isAftertouch()){
            midiMessageQueue.emplace_back(
                PPMidiMessage::Type::NoteZ,
                evtPos,
                ch,
                msg.getNoteNumber(),
                (float)msg.getAfterTouchValue() / 127.f
            );
        }
        else if(msg.isChannelPressure()){
            midiMessageQueue.emplace_back(
                PPMidiMessage::Type::Z,
                evtPos,
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
                        evtPos,
                        ch,
                        val
                    );
                    break;
                case 74:
                    midiMessageQueue.emplace_back(
                        PPMidiMessage::Type::Y,
                        evtPos,
                        ch,
                        val
                    );
                    break;
                case 1:
                    midiMessageQueue.emplace_back(
                        PPMidiMessage::Type::ModWheel,
                        evtPos,
                        ch,
                        val
                    );
                    break;
                case 2:
                    midiMessageQueue.emplace_back(
                        PPMidiMessage::Type::Breath,
                        evtPos,
                        ch,
                        val
                    );
                    break;
                case 11:
                    midiMessageQueue.emplace_back(
                        PPMidiMessage::Type::Expression,
                        evtPos,
                        ch,
                        val
                    );
                    break;
                default:
                    break;
            }
        }
    }

    parameters.visitHandleParameterValues(synth, effect);
    bufferingProcessor.process(buffer, midiMessageQueue, (float)getSampleRate(), synth, effect, getPlayHead());

    midiMessages.clear();

}

bool PhasePhckrProcessor::hasEditor() const {
    return true;
}

void extractExtra(AudioProcessorEditor *ed, PhasePhckrProcessor::InstanceSpecificPeristantState& ex) {
    if (!ed) return;
    ex.width = ed->getWidth();
    ex.height = ed->getHeight();
}

void applyExtra(AudioProcessorEditor *ed, const PhasePhckrProcessor::InstanceSpecificPeristantState& ex) {
    if (!ed) return;
    ed->setBoundsConstrained(Rectangle<int>(ex.width, ex.height));
}

AudioProcessorEditor* PhasePhckrProcessor::createEditor() {
    auto ed = new PhasePhckrEditor(*this);
    applyExtra(ed, extra);
    return ed;
}

void to_json(json& j, const PhasePhckrProcessor::InstanceSpecificPeristantState& e) {
    j["gui_width"] = e.width;
    j["gui_height"] = e.height;
}

void from_json(const json& j, PhasePhckrProcessor::InstanceSpecificPeristantState& e) {
    if(j.count("gui_width")) e.width = j["gui_width"];
    if (j.count("gui_height")) e.height = j["gui_height"];
}

void PhasePhckrProcessor::getStateInformation (MemoryBlock& destData) {
    auto p = getPreset();
    extractExtra(getActiveEditor(), extra);
    storeState(p, destData, extra);
}

void PhasePhckrProcessor::setStateInformation (const void* data, int sizeInBytes) {
    PresetDescriptor preset;
    nlohmann::json extra_j;
    loadState(data, sizeInBytes, preset, extra_j);
    setPreset(preset);
    extra = extra_j;
    applyExtra(getActiveEditor(), extra);
}

const PhasePhckr::Synth* PhasePhckrProcessor::getSynth() const {
    return synth;
}

const PhasePhckr::Effect* PhasePhckrProcessor::getEffect() const {
    return effect;
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

    if (type == SynthGraphType::VOICE) patch = voiceChain;
    else if (type == SynthGraphType::EFFECT) patch = effectChain;

    if (extractParameters) patch.parameters = getParameters(type);

    std::set<string> usedTypes = patch.root.graph.getTypes();
    patch.componentBundle.appendToUnion(componentRegister.all(), usedTypes);

    patch.cleanUp();

    return patch;
}

void PhasePhckrProcessor::setPatch(SynthGraphType type, const PatchDescriptor& patch) {
    auto patchCopy = patch;
    patchCopy.cleanUp();
    patchCopy.componentBundle.reduceToComplement(componentRegister.all());

    if (type == SynthGraphType::VOICE) {
        setVoiceChain(patchCopy);
        subVoiceChain.set(activeVoiceHandle, patchCopy);
    }
    else if (type == SynthGraphType::EFFECT) {
        setEffectChain(patchCopy);
        subEffectChain.set(activeEffectHandle, patchCopy);
    }
}

PresetDescriptor PhasePhckrProcessor::getPreset() {
    PresetDescriptor preset;

    preset.voice = getPatch(SynthGraphType::VOICE);
    preset.effect = getPatch(SynthGraphType::EFFECT);
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
    setPatch(SynthGraphType::VOICE, preset.voice);
    setPatch(SynthGraphType::EFFECT, preset.effect);
    setSettings(preset.settings);
    subSettings.set(activeSettingsHandle, activeSettings);
    parameters.deserialize(preset.parameters);
}

void PhasePhckrProcessor::setSettings(const PhasePhckr::PresetSettings &s) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();

    synth->applySettings(s);

    activeSettings = s;

    forceStateBump();
}

void PhasePhckrProcessor::setVoiceChain(const PhasePhckr::PatchDescriptor &p) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();

    json j = p;
    auto hash = std::hash<string>{}( j.dump() );
    if (hash == voiceHash) return;
    voiceHash = hash;

    voiceChain = p;
    auto pv = synth->setPatch(voiceChain, componentRegister);
    parameters.setParametersHandleMap(SynthGraphType::VOICE, pv);

    forceStateBump();
}

void PhasePhckrProcessor::setEffectChain(const PhasePhckr::PatchDescriptor &p) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();

    json j = p;
    auto hash = std::hash<string>{}(j.dump());
    if (hash == effectHash) return;
    effectHash = hash;

    effectChain = p;
    auto pv = effect->setPatch(effectChain, componentRegister);
    parameters.setParametersHandleMap(SynthGraphType::EFFECT, pv);

    forceStateBump();
}

void PhasePhckrProcessor::updateLayout(SynthGraphType type, const string &component, const map<string, ModulePosition> &layout) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();

    auto& p = type == SynthGraphType::VOICE ? voiceChain : effectChain;

    if (component == "root") {
        p.root.graph.layout = layout;
    }
    else {
        if (!p.componentBundle.has(component)) {
            // if layout is changed for a global component, add a local copy
            ComponentDescriptor cd;
            if (false == componentRegister.getComponent(component, cd)) return;
            if (p.componentBundle.set(component, cd)) return;
        }
        if(p.componentBundle.setLayout(component, layout)) return;
    }

    auto& sp = type == SynthGraphType::VOICE ? subVoiceChain : subEffectChain;
    auto& sph = type == SynthGraphType::VOICE ? activeVoiceHandle : activeEffectHandle;
    sp.set(sph, p);

    forceStateBump();
}

void PhasePhckrProcessor::forceStateBump() {
    // hack, as updateHostDisplay() doesn't work for Reaper
    Parameter* pa = nullptr;
    parameters.accessParameter(0, &pa);
    if (pa != nullptr) pa->setValueNotifyingHost(*pa);

    updateHostDisplay();
}