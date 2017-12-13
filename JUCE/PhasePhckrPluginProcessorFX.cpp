#include <phasephckr_json.hpp>
#include <xmmintrin.h>

#include "PhasePhckrPluginProcessorFX.h"
#include "PhasePhckrPluginEditorFX.h"
#include "FileIO.hpp"

#include "PhasePhckrPluginCommon.h"

using namespace PhasePhckrFileStuff;
using namespace std;

PhasePhckrProcessorFX::PhasePhckrProcessorFX()
    : AudioProcessor(BusesProperties().withOutput("Output", AudioChannelSet::stereo(), true).withInput("Input", AudioChannelSet::stereo(), true))
    , fileThings(subComponentRegister)
{
    activeEffectHandle = subEffectChain.subscribe([this](const PhasePhckr::PatchDescriptor& e){
        setEffectChain(e);
    });
    componentRegisterHandle = subComponentRegister.subscribe([this](const PhasePhckr::ComponentRegister& cr){ 
        setComponentRegister(cr);
        setEffectChain(effectChain);
    });

    createInitialUserLibrary(componentRegister); // TODO, only do this on FIRST start

    // parameter mumbo
    parameters.initialize(this);

    // create the synth and push down the initial chains
    effect = new PhasePhckr::Effect();
    setPatch(getExampleEffectChain());
}

PhasePhckrProcessorFX::~PhasePhckrProcessorFX()
{
    subEffectChain.unsubscribe(activeEffectHandle);
    subComponentRegister.unsubscribe(componentRegisterHandle);
    delete effect;
}

const String PhasePhckrProcessorFX::getName() const
{
    return JucePlugin_Name;
}

bool PhasePhckrProcessorFX::acceptsMidi() const
{
    return true;
}

bool PhasePhckrProcessorFX::producesMidi() const
{
    return false;
}

double PhasePhckrProcessorFX::getTailLengthSeconds() const
{
    return 0.0;
}

int PhasePhckrProcessorFX::getNumPrograms()
{
    return 1;
}

int PhasePhckrProcessorFX::getCurrentProgram()
{
    return 0;
}

void PhasePhckrProcessorFX::setCurrentProgram (int index)
{
}

const String PhasePhckrProcessorFX::getProgramName (int index)
{
    return String();
}

void PhasePhckrProcessorFX::changeProgramName (int index, const String& newName)
{
}

void PhasePhckrProcessorFX::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void PhasePhckrProcessorFX::releaseResources()
{
}

bool PhasePhckrProcessorFX::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::stereo()) return false;
    if (layouts.getMainInputChannelSet() != AudioChannelSet::stereo()) return false;
    return true;
}

void PhasePhckrProcessorFX::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
#if FORCE_FTZ_DAZ
    int oldMXCSR = _mm_getcsr(); /*read the old MXCSR setting */ \
    int newMXCSR = oldMXCSR | 0x8040; /* set DAZ and FZ bits */ \
    _mm_setcsr( newMXCSR); /*write the new MXCSR setting to the MXCSR */
#endif

    auto l = synthUpdateLock.make_scoped_lock();

    const int numOutputChannels = getTotalNumOutputChannels();
    const int blockSize = buffer.getNumSamples();
    const float sampleRate = (float)getSampleRate();

    // handle the parameter values
    parameters.visitHandleParameterValues(effect);

    // handle HOST properties
    auto playHead = getPlayHead();
    if(playHead){
        handlePlayHead(effect, playHead, blockSize, sampleRate, barPosition);
    }

    effect->update(buffer.getWritePointer(0), buffer.getWritePointer(1), blockSize, sampleRate);

#if FORCE_FTZ_DAZ
    _mm_setcsr( oldMXCSR );
#endif
}

bool PhasePhckrProcessorFX::hasEditor() const
{
    return true;
}

AudioProcessorEditor* PhasePhckrProcessorFX::createEditor()
{
    return new PhasePhckrEditorFX (*this);
}

void PhasePhckrProcessorFX::getStateInformation (MemoryBlock& destData)
{
    json j = getPreset();
    string ss = j.dump(2); // json lib bugged with long rows
    const char* s = ss.c_str();
    size_t n = (strlen(s) + 1) / sizeof(char);
    destData.fillWith(0);
    destData.insert((const void*)s, n, 0);
    assert(destData.getSize() == n);
}

void PhasePhckrProcessorFX::setStateInformation (const void* data, int sizeInBytes)
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

const PhasePhckr::Effect* PhasePhckrProcessorFX::getEffect() const {
    return effect;
}

void PhasePhckrProcessorFX::broadcastPatch() {
    // editor should call this once after construction
    subComponentRegister.set(componentRegisterHandle, componentRegister);
    subEffectChain.set(activeEffectHandle, effectChain);
}

PatchDescriptor PhasePhckrProcessorFX::getPatch() {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();
    auto presetParams = parameters.serialize();
    vector<PatchParameterDescriptor> params;
    for (const auto& ppd : presetParams) {
        if (ppd.type == EFFECT) {
            auto pd = ppd.p;
            params.emplace_back(pd);
        }
    }
    PatchDescriptor e = effectChain;
    e.parameters = params;
    return e;
}

PresetDescriptor PhasePhckrProcessorFX::getPreset() {
    PresetDescriptor preset;

    preset.effect = getPatch();
    preset.parameters = parameters.serialize();

    return preset;
}

void PhasePhckrProcessorFX::setComponentRegister(const ComponentRegister& cr) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();
    componentRegister = cr;
}

void PhasePhckrProcessorFX::setPreset(const PresetDescriptor& preset) {
    setPatch(preset.effect);
    parameters.deserialize(preset.parameters);
}

void PhasePhckrProcessorFX::setPatch(const PatchDescriptor& patch) {
    auto patchCopy = patch;
    patchCopy.cleanUp();
    setEffectChain(patchCopy);
    subEffectChain.set(activeEffectHandle, patchCopy);
}

void PhasePhckrProcessorFX::setEffectChain(const PhasePhckr::PatchDescriptor &p) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();
    effectChain = p;
    auto pv = effect->setEffectChain(effectChain, componentRegister);
    parameters.setParametersHandleMap(EFFECT, pv);
    updateHostDisplay();
}

void PhasePhckrProcessorFX::updateLayout(const string &component, const map<string, ModulePosition> &layout) {
    auto scoped_lock = synthUpdateLock.make_scoped_lock();

    auto* c = component == "root" ? &effectChain.root : nullptr;
    if (effectChain.components.count(component)) c = &effectChain.components[component];
    if (c == nullptr) return;
    c->layout = layout;

    // hack, as updateHostDisplay() doesn't work for Reaper
    PhasePhckrParameter* pa = nullptr;
    parameters.accessParameter(0, &pa);
    if (pa != nullptr) pa->setValueNotifyingHost(*pa);

    updateHostDisplay();
}