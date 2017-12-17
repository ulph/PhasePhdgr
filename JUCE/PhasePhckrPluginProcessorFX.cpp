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

    inputBuffer[0] = new float[Synth::internalBlockSize()]{ 0.0f };
    inputBuffer[1] = new float[Synth::internalBlockSize()]{ 0.0f };
    outputBuffer[0] = new float[Synth::internalBlockSize()]{ 0.0f };
    outputBuffer[1] = new float[Synth::internalBlockSize()]{ 0.0f };

}

PhasePhckrProcessorFX::~PhasePhckrProcessorFX()
{
    subEffectChain.unsubscribe(activeEffectHandle);
    subComponentRegister.unsubscribe(componentRegisterHandle);
    delete effect;
    delete[] inputBuffer[0];
    delete[] inputBuffer[1];
    delete[] outputBuffer[0];
    delete[] outputBuffer[1];
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
    /*
        fill up (as many samples we can) from _buffer_ starting at _inputBufferSamples_, and if full
            process _inputBuffer_ in place
            process as many remaining multiple _internalBlockSize_ of _buffer_ in-place as we can
            make sure _scratchBuffer_ is at least _blockSize_ big
            fill up _scratchBuffer_ with the data processed in _buffer_
            copy any remaining unprocessed samples in _buffer_ to _inputBuffer_, set _inputBufferSamples_

        copy _outputBufferSamples_ from _outputBuffer_ to start of _buffer_ as far as possible, set _outputBufferSamples_, _bufferSamples_
        if _bufferSamples_ < _blockSize_
            fill upp _buffer_ with _scratchBuffer_ as far possible
        top up _outputBuffer_ with any unused processed samples in _scratchBuffer_, set _outputBufferSamples_

        assert neither _inputBufferSamples_ nor _outputBufferSamples_ > _internalBlockSize_
        in fact; assert _inputBufferSamples_ + _outputBufferSamples_ <= _internalBlockSize_
        assert _bufferSamples_ == _blockSize_

        ... alterantively, use a smaller scratch buffer (same as internal block size?) do all the buffer shuffling piece wise (should be possible)
        ... or, alternatively still -- use much larger input and output buffers and do not use the scratch buffer (have to be arbitrary size though)
    */

    // pre checks
    assert(buffer.getNumChannels() >= 2);

    if (scratchBuffer.getNumChannels() != buffer.getNumChannels() || scratchBuffer.getNumSamples() != 2*buffer.getNumSamples()) {
        scratchBuffer.setSize(buffer.getNumChannels(), 2 * buffer.getNumSamples(), true, true, true);
    }

    parameters.visitHandleParameterValues(effect);

    const int blockSize = buffer.getNumSamples();
    const int internalBlockSize = Effect::internalBlockSize();

    // buffer input and process, bump to scratchBuffer
    int i = 0;
    for (i; i < internalBlockSize - inputBufferSamples && i < blockSize; i++) {
        inputBuffer[0][inputBufferSamples + i] = buffer.getSample(0, i);
        inputBuffer[1][inputBufferSamples + i] = buffer.getSample(1, i);
    }
    inputBufferSamples += i;

    auto scratchBufferSize = 0;
    if (inputBufferSamples == internalBlockSize) {
        handlePlayHead(effect, getPlayHead(), inputBufferSamples, (float)getSampleRate(), barPosition);
        effect->update(inputBuffer[0], inputBuffer[1], inputBufferSamples, (float)getSampleRate());
        for (int j=0; j < inputBufferSamples; j++) {
            scratchBuffer.setSample(0, j, inputBuffer[0][j]);
            scratchBuffer.setSample(1, j, inputBuffer[1][j]);
        }
        scratchBufferSize += inputBufferSamples;
        inputBufferSamples -= internalBlockSize;

        auto alignedBlockSize = internalBlockSize * ((blockSize - i)/ internalBlockSize);
        if (alignedBlockSize) {
            handlePlayHead(effect, getPlayHead(), alignedBlockSize, (float)getSampleRate(), barPosition);
            effect->update(buffer.getWritePointer(0, i), buffer.getWritePointer(1, i), alignedBlockSize, (float)getSampleRate());
            scratchBuffer.copyFrom(0, scratchBufferSize, buffer.getReadPointer(0, i), alignedBlockSize);
            scratchBuffer.copyFrom(1, scratchBufferSize, buffer.getReadPointer(1, i), alignedBlockSize);
        }
        scratchBufferSize += alignedBlockSize;

        int toBuffer = blockSize - i - alignedBlockSize;
        for (int j = 0; j < toBuffer && inputBufferSamples + j < internalBlockSize; j++) {
            inputBuffer[0][inputBufferSamples + j] = buffer.getSample(0, alignedBlockSize + i);
            inputBuffer[1][inputBufferSamples + j] = buffer.getSample(1, alignedBlockSize + i);
        }
        inputBufferSamples += toBuffer;
    }

    // copy samples from outputBuffer and scratchBuffer, bump remainder to outputBuffer
    int toCopy = 0;
    if (outputBufferSamples + scratchBufferSize >= blockSize) {
        int o = 0;
        for (o; o < outputBufferSamples && o < blockSize; o++) {
            buffer.setSample(0, o, outputBuffer[0][o]);
            buffer.setSample(1, o, outputBuffer[1][o]);
        }
        outputBufferSamples -= o;
        for (int j = 0; j < internalBlockSize - o; j++) {
            outputBuffer[0][j] = outputBuffer[0][o + j];
            outputBuffer[1][j] = outputBuffer[1][o + j];
        }

        toCopy = (scratchBufferSize > blockSize - o) ? blockSize - o : scratchBufferSize;
        if (toCopy) {
            buffer.copyFrom(0, o, scratchBuffer.getReadPointer(0, 0), toCopy);
            buffer.copyFrom(1, o, scratchBuffer.getReadPointer(1, 0), toCopy);
        }
    }
    int toBuffer = scratchBufferSize - toCopy;
    for (int j = 0; j < toBuffer && j < outputBufferSamples + internalBlockSize; j++) {
        outputBuffer[0][outputBufferSamples + j] = scratchBuffer.getSample(0, toCopy + j);
        outputBuffer[1][outputBufferSamples + j] = scratchBuffer.getSample(1, toCopy + j);
    }
    outputBufferSamples += toBuffer;

    // post checks
    assert(inputBufferSamples <= internalBlockSize);
    assert(outputBufferSamples <= internalBlockSize);
    assert(inputBufferSamples + outputBufferSamples == internalBlockSize);

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