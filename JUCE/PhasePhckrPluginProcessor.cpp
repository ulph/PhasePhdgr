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
    activeVoiceHandle = activeVoice.subscribe([this](const PhasePhckr::PatchDescriptor& v){setVoiceChain(v);});
    activeEffectHandle = activeEffect.subscribe([this](const PhasePhckr::PatchDescriptor& e){setEffectChain(e);});
    componentRegisterHandle = subComponentRegister.subscribe([this](const PhasePhckr::ComponentRegister& cr){ /**/ });

    createDirIfNeeded(rootDir);
    createDirIfNeeded(effectsDir);
    createDirIfNeeded(voicesDir);
    createDirIfNeeded(componentsDir);
    createDirIfNeeded(patchesDir);

    // load init patches and dump to disk
    File initVoice = voicesDir.getFullPathName() + File::getSeparatorString() + "_init.json";
    File initEffect = effectsDir.getFullPathName() + File::getSeparatorString() + "_init.json";

    voiceChain = PhasePhckr::getExampleVoiceChain();
    initVoice.replaceWithText(json(voiceChain).dump(2));

    effectChain = PhasePhckr::getExampleFxChain();
    initEffect.replaceWithText(json(effectChain).dump(2));

    // dump all factory components to disk
    for (const auto &kv : componentRegister.all()) {
        const auto &type = kv.first;
        const auto &body = kv.second;
        File cmp = componentsDir.getFullPathName() + File::getSeparatorString() + type.substr(1) + ".json";
        cmp.replaceWithText(json(body).dump(2));
    }
    fileWatchThread.startThread();
    componentDirectoryWatcher.addChangeListener(&componentFilesListener);
    componentDirectoryWatcher.setDirectory(componentsDir, true, true);

    // create the synth and push down the initial chains
    synth = new PhasePhckr::Synth();

    applyVoiceChain();
    applyEffectChain();

    for (int i = 0; i < 8*16; i++) {
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

void PhasePhckrAudioProcessor::updateParameters()
{
    map<string, int> newParameterNames;
    map<int, pair<ApiType, int>> newParameterRouting;

    // clear all the names, will get set back below
    for(const auto& p: parameterNames){
        floatParameters[p.second]->clearName();
    }

    int numNewNames = 0;
    string firstNewName = "";

    // find existing parameter (by name) and update it, or add to list of new parameters if not found
    list< pair<pair<ApiType, int>, string>> newParams;
    for(const auto& kv: voiceParameters){
        string lbl = "v " + kv.first;
        auto route = make_pair(VOICE, kv.second);
        auto it = parameterNames.find(lbl);
        if(it == parameterNames.end()){
            newParams.push_back(make_pair(route, lbl));
            if(numNewNames == 0){
                firstNewName = lbl;
            }
            numNewNames++;
        }
        else{
            floatParameters[it->second]->setName(lbl);
            newParameterRouting[it->second] = route;
            newParameterNames[lbl] = it->second;
        }
    }
    for(const auto& kv: effectParameters){
        string lbl = "e " + kv.first;
        auto route = make_pair(EFFECT, kv.second);
        auto it = parameterNames.find(lbl);
        if(it == parameterNames.end()){
            newParams.push_back(make_pair(route, lbl));
            if(numNewNames == 0){
                firstNewName = lbl;
            }
            numNewNames++;
        }
        else{
            newParameterRouting[it->second] = route;
            newParameterNames[lbl] = it->second;
            floatParameters[it->second]->setName(lbl);
        }
    }

    // special case - one new name and just one less new params -> a single rename
    if(numNewNames == 1 && newParameterNames.size() == (parameterNames.size()-1)){
        for(const auto& kv: parameterNames){
            if(!newParameterNames.count(kv.first)){
                // found it!
                auto it = newParams.begin();
                while(it != newParams.end()){
                    if(it->second == firstNewName){
                        // found it also in newParams... apply and delete
                        newParameterRouting[kv.second] = it->first;
                        newParameterNames[firstNewName] = kv.second;
                        floatParameters[kv.second]->setName(firstNewName);
                        newParams.erase(it);
                        break;
                    }
                    it++;
                }
                break;
            }
        }
    }

    // for any new parameters, find first free slot and stick it there
    for(int i=0; i<floatParameters.size(); i++){
        if(newParams.size() == 0) break;
        if(newParameterRouting.count(i)) continue;
        auto p = newParams.front(); newParams.pop_front();
        newParameterRouting[i] = p.first;
        newParameterNames[p.second] = i;
        floatParameters[i]->setName(p.second);
    }

    if(newParams.size()){
        cerr << "Warning - number of parameter modules larger than number allocated in plug-in!" << endl;
    }

    // replace the old route table and name book-keeping
    parameterNames = newParameterNames;
    parameterRouting = newParameterRouting;

    assert(parameterNames.size() == parameterRouting.size());
    for(const auto& p: parameterNames){
        assert(parameterRouting.count(p.second));
    }

}

void PhasePhckrAudioProcessor::applyVoiceChain()
{
    while (synthUpdateLock.test_and_set(std::memory_order_acquire));
    voiceParameters = synth->setVoiceChain(voiceChain, componentRegister);
    updateParameters();
    synthUpdateLock.clear(std::memory_order_release);
}

void PhasePhckrAudioProcessor::applyEffectChain()
{
    while (synthUpdateLock.test_and_set(std::memory_order_acquire));
    effectParameters = synth->setFxChain(effectChain, componentRegister);
    updateParameters();
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
    for(const auto kv: parameterRouting){
        auto idx = kv.first;
        auto type = kv.second.first;
        auto handle = kv.second.second;
        auto p = floatParameters[idx];
        float value = p->range.convertFrom0to1(*p);
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
    PresetDescriptor preset;
    while (synthUpdateLock.test_and_set(std::memory_order_acquire));
    preset.voice = voiceChain;
    preset.effect = effectChain;
    preset.parameters = vector<ParameterDescriptor>();
    for(const auto &kv : parameterNames){
        auto param = floatParameters[kv.second];

        ParameterDescriptor p = {
            kv.first,
            kv.second,
            *param,
            param->range.start,
            param->range.end
        };
        preset.parameters.emplace_back(p);
    }
    synthUpdateLock.clear(std::memory_order_release);

    json j = preset;
    string ss = j.dump(2); // TODO; json lib bugged with long rows...
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

    for (const auto& fp : floatParameters) {
        fp->clearName();
        fp->setValueNotifyingHost(0.f); // I think this is the correct thing to do
    }
    for (const auto& p : preset.parameters) {
        parameterNames[p.id] = p.index;
        auto param = floatParameters[p.index];
        param->range.start = p.min;
        param->range.end = p.max;
        param->setValueNotifyingHost(param->range.convertTo0to1(p.value));
        // we could also set name but updateParameters takes care of that
    }
    activeVoice.set(-1, preset.voice);
    activeEffect.set(-1, preset.effect);
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhasePhckrAudioProcessor();
}

const PhasePhckr::Synth* PhasePhckrAudioProcessor::getSynth() const {
    return synth;
}

bool PhasePhckrAudioProcessor::accessParameter(int index, PhasePhckrParameter ** param){
    if(index >= numberOfParameters()) return false;
    *param = floatParameters[index];
    return true;
}

size_t PhasePhckrAudioProcessor::numberOfParameters(){
    return floatParameters.size();
}

void PhasePhckrAudioProcessor::swapParameterIndices(string a, string b){
    int a_idx = -1;
    int b_idx = -1;
    for(int i=0; i<floatParameters.size(); i++){
        if(floatParameters[i]->getName(64) == a){
            assert(a_idx == -1);
            a_idx = i;
        }
        if(floatParameters[i]->getName(64) == b){
            assert(b_idx == -1);
            b_idx = i;
        }
    }
    if(a_idx == b_idx) return;
    if(a_idx == -1 || b_idx == -1) return;
    if(!floatParameters[a_idx]->isActive() && !floatParameters[b_idx]->isActive()) return; // todo, should not even get this far...
    parameterNames[a] = b_idx;
    parameterNames[b] = a_idx;
    float a_val = *floatParameters[a_idx];
    float b_val = *floatParameters[b_idx];
    floatParameters[a_idx]->setValueNotifyingHost(b_val);
    floatParameters[b_idx]->setValueNotifyingHost(a_val);
    updateParameters();
}
