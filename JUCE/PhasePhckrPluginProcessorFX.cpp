#include <phasephckr_json.hpp>
#include <xmmintrin.h>

#include "PhasePhckrPluginProcessorFX.h"
#include "PhasePhckrPluginEditorFX.h"
#include "FileIO.hpp"

using namespace PhasePhckrFileStuff;
using namespace std;

void PhasePhckrProcessorFX::updateComponentRegister(const DirectoryContentsList* d)
{
    for(int i=0; i<d->getNumFiles(); i++) {
        const File& f = d->getFile(i);
        if(!f.existsAsFile()) continue; // TODO, recurse into subdirs
        String p = f.getRelativePathFrom(componentsDir);
        string n = string(&componentMarker, 1) + p.dropLastCharacters(5).toUpperCase().toStdString(); // remove .json
        string s = f.loadFileAsString().toStdString();
        try {
            json j = json::parse(s.c_str());
            ComponentDescriptor cd = j;
            componentRegister.registerComponent(n, cd);
            subComponentRegister.set(-1, componentRegister);
        } catch (const std::exception& e) {
            (void)e;
            continue;
            assert(0);
        }
    }
}

PhasePhckrProcessorFX::PhasePhckrProcessorFX()
    : AudioProcessor(BusesProperties().withOutput("Output", AudioChannelSet::stereo(), true).withInput("Input", AudioChannelSet::stereo(), true))
    , fileWatchThread("processorFxFileWatchThread")
    , componentDirectoryWatcher(getFilter(), fileWatchThread)
    , componentFilesListener(StupidFileListCallBack([this](const DirectoryContentsList* d){updateComponentRegister(d);}))
{
    activeEffectHandle = subEffectChain.subscribe([this](const PhasePhckr::PatchDescriptor& e){
        setEffectChain(e);
    });
    componentRegisterHandle = subComponentRegister.subscribe([this](const PhasePhckr::ComponentRegister& cr){ 
        setEffectChain(effectChain);
    });

    createInitialUserLibrary(componentRegister); // TODO, only do this on FIRST start

    fileWatchThread.startThread();
    componentDirectoryWatcher.addChangeListener(&componentFilesListener);
    componentDirectoryWatcher.setDirectory(componentsDir, true, true);

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
    float sampleRate = (float)getSampleRate();

    // handle the parameter values
    parameters.visitHandleParameterValues(effect);

    // handle HOST properties
    auto playHead = getPlayHead();
    if(playHead){
        AudioPlayHead::CurrentPositionInfo info;
        playHead->getCurrentPosition(info);
        effect->handleTimeSignature(info.timeSigNumerator, info.timeSigDenominator);
        effect->handleBPM((float)info.bpm);
        effect->handlePosition((float)info.ppqPosition);
        effect->handleTime((float)info.timeInSeconds);
        if (info.isPlaying) {
            barPosition = (float)info.ppqPosition - (float)info.ppqPositionOfLastBarStart;
        }
        else {
            auto timedelta = (float)blockSize / sampleRate;
            auto quarterdelta = timedelta * (float)info.bpm / 60.f;
            auto barLength = 4.f * (float)info.timeSigNumerator / (float)info.timeSigDenominator;
            barPosition += quarterdelta;
            barPosition = fmod(barPosition, barLength);
        }
        effect->handleBarPosition(barPosition);
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
    json j = getPatch();
    string ss = j.dump(2); // json lib bugged with long rows
    const char* s = ss.c_str(); 
    size_t n = (strlen(s)+1) / sizeof(char);
    destData.fillWith(0);
    destData.insert((const void*)s, n, 0);
    assert(destData.getSize() == n);
}

void PhasePhckrProcessorFX::setStateInformation (const void* data, int sizeInBytes)
{
    string ss((const char*)data);
    PatchDescriptor patch;
    try {
        patch = json::parse(ss.c_str());
    }
    catch (const nlohmann::detail::exception& e) {
        auto msg = e.what();
        // sod off
        return;
    }
    setPatch(patch);
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
    return effectChain;
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
}
