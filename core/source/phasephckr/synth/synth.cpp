#include <phasephckr/synth.hpp>

#include "synthvoice.hpp"
#include "effectchain.hpp"

#if SUPPORT_PLUGIN_LOADING
#include "pluginsregister.hpp"
#endif

namespace PhasePhckr {

const float c_maxSaneValue = 5.f;
const float c_numSecondsBetweenResetAttempts = 1.0f;

Base::Base()
    : globalData(new GlobalData())
    , scopeHz(10.f)
{}

Base::~Base() {
    delete globalData;
}

int Base::internalBlockSize() {
    return ConnectionGraph::k_blockSize;
}

Effect::Effect()
    : effects(nullptr)
{
}

Effect::~Effect() {
    delete effects;
}

Synth::Synth()
    : voiceBus(new VoiceBus())
    , scopeVoiceIndex(-1)
    , concurrency(std::thread::hardware_concurrency())
    , pool(concurrency)
{
}

Synth::~Synth(){
    for(SynthVoice *v : voices){
        delete v;
    }
    delete voiceBus;
}

const ParameterHandleMap& Effect::setPatch(const PatchDescriptor& fxChain, const ComponentRegister & cp) {
    delete effects;
    effects = new EffectChain(fxChain, cp, nullptr);
    return effects->getParameterHandles();
}

const ParameterHandleMap& Effect::setPatch(const PatchDescriptor& fxChain, const ComponentRegister & cp, const SDKExtensionManager & sdk) {
    delete effects;
    effects = new EffectChain(fxChain, cp, sdk.sdkPluginRegister);
    return effects->getParameterHandles();
}

void Synth::resetVoiceBus(const SynthVoice* voice) {
    assert(settings.polyphony > 0);

    voiceBus->setLegato(settings.legatoMode);
    voiceBus->setStealPolicy(settings.getNoteStealPolicy());
    voiceBus->setReactivationPolicy(settings.getNoteReactivationPolicy());
    voiceBus->setActivationPolicy(settings.noteActivationPolicy);

    for (auto v : voices) delete v;
    voices.clear();

    auto numVoices = settings.polyphony > 0 ? settings.polyphony : 1;

    for (int i = 0; i<numVoices; ++i) {
        SynthVoice *v_ = new SynthVoice(*voice);
        v_->mpe.setIndex(i, settings.polyphony);
        voices.push_back(v_);
    }

    voiceBus->reassignVoices(voices);
}

const ParameterHandleMap& Synth::setPatch(const PatchDescriptor& voiceChain, const ComponentRegister & cp){
    SynthVoice v(voiceChain, cp, nullptr);
    v.preCompile(lastKnownSampleRate);
    resetVoiceBus(&v);
    return voices[0]->getParameterHandles(); // they're identical
}

const ParameterHandleMap& Synth::setPatch(const PatchDescriptor& voiceChain, const ComponentRegister & cp, const SDKExtensionManager & sdk) {
    SynthVoice v(voiceChain, cp, sdk.sdkPluginRegister);
    v.preCompile(lastKnownSampleRate);
    resetVoiceBus(&v);
    return voices[0]->getParameterHandles(); // they're identical
}

void Effect::update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate)
{
    inputScopeL.writeToBuffer(leftChannelbuffer, numSamples, sampleRate, scopeHz);
    inputScopeR.writeToBuffer(rightChannelbuffer, numSamples, sampleRate, scopeHz);
    effects->update(leftChannelbuffer, rightChannelbuffer, numSamples, sampleRate, *globalData);

    if (fabsf(*leftChannelbuffer) > c_maxSaneValue || fabsf(*rightChannelbuffer) > c_maxSaneValue) {
        secondsSinceReset += numSamples / sampleRate;
        if (secondsSinceReset > c_numSecondsBetweenResetAttempts) {
            secondsSinceReset = 0.0f;
            std::cerr << "abnormal sample value, resetting effect chain" << std::endl;
            effects->reset();
            for (auto i = 0; i < numSamples; i++) {
                leftChannelbuffer[i] = 0.0f;
                rightChannelbuffer[i] = 0.0f;
            }
        }
    }

    outputScopeL.writeToBuffer(leftChannelbuffer, numSamples, sampleRate, scopeHz);
    outputScopeR.writeToBuffer(rightChannelbuffer, numSamples, sampleRate, scopeHz);
}

void Synth::update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate)
{
    lastKnownSampleRate = sampleRate;

    int samplesLeft = numSamples;
    int maxChunk = SYNTH_VOICE_BUFFER_LENGTH;
    float *bufL = leftChannelbuffer, *bufR = rightChannelbuffer;

    int newScopeVoiceIndex = voiceBus->findScopeVoiceIndex(voices);
    if (newScopeVoiceIndex != -1) {
        scopeVoiceIndex = newScopeVoiceIndex;
        scopeHz = voices[scopeVoiceIndex]->mpe.getState().pitchHz[0];
    }
    else if(scopeVoiceIndex != -1 && voices[scopeVoiceIndex]->isSilent()){
        scopeVoiceIndex = -1;
    }

    while(samplesLeft > 0) {
        int chunkSize = (samplesLeft < maxChunk) ? samplesLeft : maxChunk;

        for (auto & v : voices) v->processingStart(chunkSize, sampleRate, *globalData);

        if (concurrency > 1 && settings.multicore && voices.size() > 1) {
            std::vector< std::future<void> > results;
            for (auto & v : voices) {
                results.emplace_back(
                    pool.enqueue([v] {v->threadedProcess(); })
                );
            }
            for (auto && result : results) result.get();
        }
        else {
            for (auto & v : voices) v->threadedProcess();
        }

        for (auto & v : voices) v->processingFinish(bufL, bufR, chunkSize);

        samplesLeft -= chunkSize;
        bufL += chunkSize;
        bufR += chunkSize;

        if (scopeVoiceIndex != -1) {
            voiceScopeL.writeToBuffer(voices[scopeVoiceIndex]->getInternalBuffer(0), chunkSize, sampleRate, scopeHz);
            voiceScopeR.writeToBuffer(voices[scopeVoiceIndex]->getInternalBuffer(1), chunkSize, sampleRate, scopeHz);
        }
    }

    if (fabsf(*leftChannelbuffer) > c_maxSaneValue || fabsf(*rightChannelbuffer) > c_maxSaneValue) {
        secondsSinceReset += numSamples / sampleRate;
        if (secondsSinceReset > c_numSecondsBetweenResetAttempts) {
            secondsSinceReset = 0.0f;
            std::cerr << "abnormal sample value, resetting voice chains" << std::endl;
            for (auto v : voices) v->reset();
            for (auto i = 0; i < numSamples; i++) {
                leftChannelbuffer[i] = 0.0f;
                rightChannelbuffer[i] = 0.0f;
            }
        }
    }

    outputScopeL.writeToBuffer(leftChannelbuffer, numSamples, sampleRate, scopeHz);
    outputScopeR.writeToBuffer(rightChannelbuffer, numSamples, sampleRate, scopeHz);

    voiceBus->update(voices);
}

void Synth::handleNoteOnOff(int a, int b, float c, bool d) { voiceBus->handleNoteOnOff(a, b, c, d, voices); }
void Synth::handleX(int a, float b) { voiceBus->handleX(a, b, voices); }
void Synth::handleY(int a, float b) { voiceBus->handleY(a, b, voices); }
void Synth::handleZ(int a, float b) { voiceBus->handleZ(a, b, voices); }
void Synth::handleNoteZ(int a, int b, float c) { voiceBus->handleNoteZ(a, b, c, voices); }
void Synth::handleSustain(float b) { voiceBus->handleSustain(b, voices); }
void Synth::handleExpression(float a) { globalData->expression(a); }
void Synth::handleBreath(float a) { globalData->breath(a); }
void Synth::handleModWheel(float a) { globalData->modwheel(a); }

void Base::handleTimeSignature(int num, int den){ globalData->signature(num, den); }
void Base::handleBPM(float bpm){ globalData->bpm(bpm); }
void Base::handlePosition(float ppqPosition){ globalData->position(ppqPosition); }
void Base::handleBarPosition(float ppqPosition){ globalData->barPosition(ppqPosition); }
void Base::handleTime(float t){ globalData->time(t); }

void Effect::handleParameter(int handle, float value){
    effects->setParameter(handle, value);
}

void Synth::handleParameter(int handle, float value){
    for(auto & v : voices) v->setParameter(handle, value);
}

const Scope& Synth::getVoiceScope(int i) const {
    if(i==0) return voiceScopeL;
    else if(i==1) return voiceScopeR;
    return voiceScopeL;
}

const Scope& Base::getOutputScope(int i) const {
    if(i==0) return outputScopeL;
    else if(i==1) return outputScopeR;
    return outputScopeL;
}

const Scope& Base::getVoiceScope(int i) const {
    return getOutputScope(i);
}

const Scope& Base::getInputScope(int i) const {
    return getOutputScope(i);
}

const Scope& Effect::getInputScope(int i) const {
    if (i == 0) return inputScopeL;
    else if (i == 1) return inputScopeR;
    return inputScopeL;
}

const PresetSettings& Synth::retrieveSettings() {
    return settings;
}

void Synth::applySettings(const PresetSettings& newSettings) {
    settings = newSettings;
    if (voices.size()) {
        auto v = *voices[0];
        v.mpe.reset();
        resetVoiceBus(&v);
    }
}

SDKExtensionManager::SDKExtensionManager()
#if SUPPORT_PLUGIN_LOADING
    : sdkPluginRegister(new PluginsRegister())
#endif
{}

SDKExtensionManager::~SDKExtensionManager() {
#if SUPPORT_PLUGIN_LOADING
    delete sdkPluginRegister;
#endif
}

void SDKExtensionManager::registerSdkExtensions(const std::set<std::string>& filenames) {
#if SUPPORT_PLUGIN_LOADING
    for (const auto& fname : filenames) {
        sdkPluginRegister->loadPlugin(fname.c_str());
    }
#endif
}

void SDKExtensionManager::updateDoc(Doc* doc) {
#if SUPPORT_PLUGIN_LOADING
    auto cg = ConnectionGraph();
    sdkPluginRegister->registerModules(&cg);
    std::vector<ModuleDoc> dd;
    cg.makeModuleDocs(dd);
    for (const auto & d : dd) {
        doc->add(d);
    }
#endif
}

}
