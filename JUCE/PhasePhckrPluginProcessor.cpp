#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrPluginEditor.h"
#include "design_json.hpp"

#include <xmmintrin.h>

using namespace PhasePhckrFileStuff;
using namespace std;

void PhasePhckrAudioProcessor::updateComponentRegister(const DirectoryContentsList* d)
{
    for(int i=0; i<d->getNumFiles(); i++) {
        const File& f = d->getFile(i);
        if(!f.existsAsFile()) continue; // TODO, recurse into subdirs
        String p = f.getRelativePathFrom(componentsDir);
        string n = string("@")+p.dropLastCharacters(5).toUpperCase().toStdString(); // remove .json
        string s = f.loadFileAsString().toStdString();
        try {
            json j = json::parse(s.c_str());
            ComponentDescriptor cd = j;
            componentRegister.registerComponent(n, cd);
            subComponentRegister.set(componentRegisterHandle, componentRegister);
        } catch (const std::exception& e) {
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
    fileWatchThread.startThread();
    componentDirectoryWatcher.addChangeListener(&componentFilesListener);
    componentDirectoryWatcher.setDirectory(componentsDir, true, true);

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

void PhasePhckrAudioProcessor::updateParameters(bool newVoiceChain, bool newEffectChain)
{
    // clean
    list<int> toPrune;
    for(const auto& kv : parameterRouting){
        if((kv.second.first == VOICE && newVoiceChain) || (kv.second.first == EFFECT && newEffectChain))
            toPrune.push_back(kv.first);
    }
    for(const auto& i: toPrune){
        floatParameters[i]->clearLabel();
        parameterRouting.erase(i);
    }

    // create a joblist of all new params
    list< pair<pair<ApiType, int>, string>> newParams;
    if(newVoiceChain){
        for(const auto& kv: voiceParameters){
            newParams.push_back(make_pair(make_pair(VOICE, kv.second), kv.first));
        }
    }
    if(newEffectChain){
        for(const auto& kv: effectParameters){
            newParams.push_back(make_pair(make_pair(EFFECT, kv.second), kv.first));
        }
    }

    // find first free slot and stick it there
    for(int i=0; i<floatParameters.size(); i++){
        if(newParams.size() == 0) break;
        if(parameterRouting.count(i)) continue;
        auto p = newParams.front(); newParams.pop_front();
        parameterRouting[i] = p.first;
        floatParameters[i]->setLabel(p.second);
    }

}

void PhasePhckrAudioProcessor::applyVoiceChain()
{
    while (synthUpdateLock.test_and_set(std::memory_order_acquire));
    voiceParameters = synth->setVoiceChain(voiceChain, componentRegister);
    updateParameters(true, false);
    synthUpdateLock.clear(std::memory_order_release);
}

void PhasePhckrAudioProcessor::applyEffectChain()
{
    while (synthUpdateLock.test_and_set(std::memory_order_acquire));
    effectParameters = synth->setFxChain(effectChain, componentRegister);
    updateParameters(false, true);
    synthUpdateLock.clear(std::memory_order_release);
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

    // handle the parameter values
    for(const auto kv: parameterRouting){
        auto idx = kv.first;
        auto type = kv.second.first;
        auto handle = kv.second.second;
        float value = *(floatParameters[idx]);
        switch(type){
        case VOICE:
            synth->setVoiceParameter(handle, value);
            break;
        case EFFECT:
            synth->setFxParameter(handle, value);
            break;
        default:
            break;
        }
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
    json j;
    while (synthUpdateLock.test_and_set(std::memory_order_acquire));
    j["voice"] = voiceChain;
    j["effect"] = effectChain;
    synthUpdateLock.clear(std::memory_order_release);
    auto ss = j.dump(2);
    const char* s = ss.c_str();
    size_t n = (strlen(s)+1) / sizeof(char);
    destData.fillWith(0);
    destData.insert((const void*)s, n, 0);
    assert(destData.getSize() == n);
}

void PhasePhckrAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    string ss((const char*)data);
    json j;
    try {
        j = json::parse(ss.c_str());
    } catch (const std::exception& e) {
        assert(0);
    }
    PatchDescriptor newVoiceChain = j.at("voice").get<PatchDescriptor>();
    PatchDescriptor newEffectChain = j.at("effect").get<PatchDescriptor>();
    activeVoice.set(-1, newVoiceChain);
    activeEffect.set(-1, newEffectChain);
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhasePhckrAudioProcessor();
}

const PhasePhckr::Synth* PhasePhckrAudioProcessor::getSynth() const {
    return synth;
}
