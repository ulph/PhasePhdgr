#include <phasephdgr_json.hpp>
#include <xmmintrin.h>

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FileIO.hpp"

#include "PluginCommon.h"

using namespace PhasePhdgrFileStuff;
using namespace std;

void PhasePhdgrProcessorBase::initialize() {

    activeSettingsHandle = subSettings.subscribe([this](const PhasePhdgr::PresetSettings& s) {
        setSettings(s);
    });

    for (auto &kv : bundles) {
        bundles[kv.first].handle = bundles[kv.first].propagator.subscribe([this, type = kv.first](const PhasePhdgr::PatchDescriptor& p) {
            setPatch(type, p);
        });
    }
    componentRegisterHandle = subComponentRegister.subscribe([this](const PhasePhdgr::ComponentRegister& cr) {
        setComponentRegister(cr);
        for (auto &kv : bundles) setPatch(kv.first, kv.second.patch);
    });

    createInitialUserLibrary(componentRegister);

    componentLoader.rescan();

    // extension loading hax
    std::set<std::string> plugins;
    auto sdk_dir = PhasePhdgrFileStuff::sdkExtensionsDir;
    for (const auto & fname : sdk_dir.findChildFiles(File::findFiles, false, "*.ppp.*")) {
        plugins.insert(fname.getFullPathName().toStdString());
    }
    sdkExtensionManager.registerSdkExtensions(plugins);
    // end of extension loading hax

    parameters.initialize(this);
}

void PhasePhdgrProcessorBase::destroy() {
    for (auto &kv : bundles) kv.second.unsubscribe();
    subComponentRegister.unsubscribe(componentRegisterHandle);
}

const String PhasePhdgrProcessorBase::getName() const
{
    return JucePlugin_Name;
}

double PhasePhdgrProcessorBase::getTailLengthSeconds() const
{
    return 0.0;
}

int PhasePhdgrProcessorBase::getNumPrograms()
{
    return 1;
}

int PhasePhdgrProcessorBase::getCurrentProgram()
{
    return 0;
}

void PhasePhdgrProcessorBase::setCurrentProgram (int index)
{
}

const String PhasePhdgrProcessorBase::getProgramName (int index)
{
    return String();
}

void PhasePhdgrProcessorBase::changeProgramName (int index, const String& newName)
{
}

void PhasePhdgrProcessorBase::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void PhasePhdgrProcessorBase::releaseResources()
{
}

bool PhasePhdgrProcessorBase::hasEditor() const {
    return true;
}

void to_json(json& j, const PhasePhdgrProcessorBase::InstanceSpecificPeristantState& e) {
    j["gui_width"] = e.width;
    j["gui_height"] = e.height;
}

void from_json(const json& j, PhasePhdgrProcessorBase::InstanceSpecificPeristantState& e) {
    if(j.count("gui_width")) e.width = j["gui_width"];
    if (j.count("gui_height")) e.height = j["gui_height"];
}

void extractExtra(AudioProcessorEditor *ed, PhasePhdgrProcessorBase::InstanceSpecificPeristantState& ex) {
    if (!ed) return;
    ex.width = ed->getWidth();
    ex.height = ed->getHeight();
}

void applyExtra(AudioProcessorEditor *ed, const PhasePhdgrProcessorBase::InstanceSpecificPeristantState& ex) {
    if (!ed) return;
    ed->setBoundsConstrained(Rectangle<int>(ex.width, ex.height));
}

void PhasePhdgrProcessorBase::getStateInformation (MemoryBlock& destData) {
    auto p = getPreset();
    extractExtra(getActiveEditor(), extra);
    storeState(p, destData, extra);
}

void PhasePhdgrProcessorBase::setStateInformation (const void* data, int sizeInBytes) {
    PresetDescriptor preset;
    nlohmann::json extra_j;
    loadState(data, sizeInBytes, preset, extra_j);
    setPreset(preset);
    extra = extra_j;
    applyExtra(getActiveEditor(), extra);
}

const PhasePhdgr::Base* PhasePhdgrProcessorBase::getProcessor(SynthGraphType type) const {
    if (!bundles.count(type)) return nullptr;
    return bundles.at(type).processor;
}

void PhasePhdgrProcessorBase::broadcastPatch() {
    // editor should call this once after construction
    subComponentRegister.set(componentRegisterHandle, componentRegister);
    for (const auto &b : bundles) b.second.broadcast();
    subSettings.set(activeSettingsHandle, activeSettings);
}

PatchDescriptor PhasePhdgrProcessorBase::getPatch(SynthGraphType type, bool extractParameters) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();

    PatchDescriptor patch; 
    
    if(bundles.count(type)) patch = bundles.at(type).patch;

    if (extractParameters) patch.parameters = getParameters(type);

    std::set<string> usedTypes = patch.root.graph.getTypes();
    patch.componentBundle.appendToUnion(componentRegister.all(), usedTypes);

    patch.cleanUp();

    return patch;
}

PresetDescriptor PhasePhdgrProcessorBase::getPreset() {
    PresetDescriptor preset;

    preset.voice = getPatch(SynthGraphType::VOICE);
    preset.effect = getPatch(SynthGraphType::EFFECT);
    preset.parameters = parameters.serialize();
    preset.settings = activeSettings;

    return preset;
}

vector<PresetParameterDescriptor> PhasePhdgrProcessorBase::getPresetParameters() {
    return parameters.serialize();
}

vector<PatchParameterDescriptor> PhasePhdgrProcessorBase::getParameters(SynthGraphType type) {
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

void PhasePhdgrProcessorBase::setComponentRegister(const ComponentRegister& cr) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();
    componentRegister = cr;
}

void PhasePhdgrProcessorBase::setPreset(const PresetDescriptor& preset) {
    setPatch(SynthGraphType::VOICE, preset.voice, true);
    setPatch(SynthGraphType::EFFECT, preset.effect, true);
    setSettings(preset.settings);
    subSettings.set(activeSettingsHandle, activeSettings);
    parameters.deserialize(preset.parameters);
}

void PhasePhdgrProcessorBase::setSettings(const PhasePhdgr::PresetSettings &s) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();

    for (auto &kv: bundles) {
        if (!kv.second.processor) continue;
        kv.second.processor->applySettings(s);
    }

    activeSettings = s;

    forceStateBump();
}

void PhasePhdgrProcessorBase::setPatch(SynthGraphType type, const PhasePhdgr::PatchDescriptor &patch, bool cleanLocalComponentDupes) {
    if (!bundles.count(type)) return;
    auto scoped_lock = synthUpdateLock.make_scoped_lock();
    auto &bundle = bundles[type];
    if (!bundle.processor) return;

    auto p = patch;
    p.cleanUp();
    if(cleanLocalComponentDupes) {
        p.componentBundle.reduceToComplement(componentRegister.all());
    }

    json j = p;
    auto hash = std::hash<string>{}( j.dump() );
    if (hash == bundle.hash) return;

    bundle.hash = hash;
    bundle.patch = p;
    auto pv = bundle.processor->setPatch(bundle.patch, componentRegister, sdkExtensionManager);
    parameters.setParametersHandleMap(type, pv);
    bundle.broadcast();

    forceStateBump();
}

void PhasePhdgrProcessorBase::updateLayout(SynthGraphType type, const string &component, const map<string, ModulePosition> &layout) {
    if (!bundles.count(type)) return;

    auto scoped_lock = synthUpdateLock.make_scoped_lock();

    auto &bundle = bundles[type];

    auto& p = bundle.patch;

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

    bundle.broadcast();

    forceStateBump();
}

SubValue<PatchDescriptor> & PhasePhdgrProcessorBase::getPropagator(SynthGraphType type) {
    // TODO, get rid of this access
    if (!bundles.count(type)) return nullPatchPropagator;
    return bundles[type].propagator;
}

void PhasePhdgrProcessorBase::forceStateBump() {
    // hack, as updateHostDisplay() doesn't work for Reaper
    Parameter* pa = nullptr;
    parameters.accessParameter(0, &pa);
    if (pa != nullptr) pa->setValueNotifyingHost(*pa);

    updateHostDisplay();
}

// synth

PhasePhdgrProcessor::PhasePhdgrProcessor()
    : PhasePhdgrProcessorBase(BusesProperties().withOutput("Output", AudioChannelSet::stereo(), true).withInput("Input", AudioChannelSet::disabled(), true))
{

    midiMessageQueue.reserve(128); // some nice large-ish number

    synth = new PhasePhdgr::Synth();
    effect = new PhasePhdgr::Effect();

    bundles[SynthGraphType::VOICE].processor = synth;
    bundles[SynthGraphType::EFFECT].processor = effect;

    initialize();

    PresetDescriptor initialPreset;
    initialPreset.voice = getExampleVoiceChain();
    initialPreset.effect = getExampleEffectChain();

    setPreset(initialPreset);
}

PhasePhdgrProcessor::~PhasePhdgrProcessor()
{
    destroy();
    delete synth;
    delete effect;
}

AudioProcessorEditor* PhasePhdgrProcessor::createEditor() {
    auto ed = new PhasePhdgrEditor(*this);
    applyExtra(ed, extra);
    return ed;
}

bool PhasePhdgrProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::stereo()) return false;
    return true;
}

bool PhasePhdgrProcessor::acceptsMidi() const
{
    return true;
}

bool PhasePhdgrProcessor::producesMidi() const
{
    return false;
}

void PhasePhdgrProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
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
    while (midiIt.getNextEvent(msg, evtPos)) {
        int ch = msg.getChannel();
        if (msg.isNoteOnOrOff()) {
            midiMessageQueue.emplace_back(
                msg.isNoteOn(true) ? PPMidiMessage::Type::On : PPMidiMessage::Type::Off,
                evtPos,
                ch,
                msg.getNoteNumber(),
                msg.getFloatVelocity()
            );
        }
        else if (msg.isPitchWheel()) {
            midiMessageQueue.emplace_back(
                PPMidiMessage::Type::X,
                evtPos,
                ch,
                2.f*((float)msg.getPitchWheelValue() / (float)(0x3fff) - 0.5f)
            );
        }
        else if (msg.isAftertouch()) {
            midiMessageQueue.emplace_back(
                PPMidiMessage::Type::NoteZ,
                evtPos,
                ch,
                msg.getNoteNumber(),
                (float)msg.getAfterTouchValue() / 127.f
            );
        }
        else if (msg.isChannelPressure()) {
            midiMessageQueue.emplace_back(
                PPMidiMessage::Type::Z,
                evtPos,
                ch,
                (float)msg.getChannelPressureValue() / 127.f
            );
        }
        else if (msg.isController()) {
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

const String PhasePhdgrProcessor::getName() const {
    return "phasepckr";
}

// effect

PhasePhdgrProcessorFx::PhasePhdgrProcessorFx()
    : PhasePhdgrProcessorBase(BusesProperties().withOutput("Output", AudioChannelSet::stereo(), true).withInput("Input", AudioChannelSet::stereo(), true))
{
    effect = new PhasePhdgr::Effect();

    bundles[SynthGraphType::EFFECT].processor = effect;

    initialize();

    PresetDescriptor initialPreset;
    initialPreset.effect = getExampleEffectChain();

    setPreset(initialPreset);
}

PhasePhdgrProcessorFx::~PhasePhdgrProcessorFx()
{
    destroy();
    delete effect;
}

AudioProcessorEditor* PhasePhdgrProcessorFx::createEditor() {
    auto ed = new PhasePhdgrEditorFX(*this);
    applyExtra(ed, extra);
    return ed;
}

bool PhasePhdgrProcessorFx::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::stereo()) return false;
    if (layouts.getMainInputChannelSet() != AudioChannelSet::stereo()) return false;
    return true;
}

bool PhasePhdgrProcessorFx::acceptsMidi() const
{
    return false;
}

bool PhasePhdgrProcessorFx::producesMidi() const
{
    return false;
}

void PhasePhdgrProcessorFx::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    auto dn = ScopedNoDenormals();
    auto l = synthUpdateLock.make_scoped_lock();   
    bufferingProcessor.process(buffer, (float)getSampleRate(), effect, getPlayHead());
}

const String PhasePhdgrProcessorFx::getName() const {
    return "phasepckrfx";
}
