#include "PluginCommon.h"
#include "Parameters.hpp"

void storeState(const PresetDescriptor& preset, MemoryBlock& destData) {
    json j = preset;
    string ss = j.dump(2); // json lib bugged with long rows
    const char* s = ss.c_str();
    size_t n = (strlen(s) + 1) / sizeof(char);
    destData.fillWith(0);
    destData.insert((const void*)s, n, 0);
    assert(destData.getSize() == n);
}

void loadState(const void* data, int sizeInBytes, PresetDescriptor& preset) {
    string ss((const char*)data);
    try {
        preset = json::parse(ss.c_str());
    }
    catch (const nlohmann::detail::exception& e) {
        auto msg = e.what();
        cerr << "loadState: " << msg << endl;
        return;
    }
}

void handlePlayHead(Effect* effect, AudioPlayHead* playHead, const int blockSize, const float sampleRate, float& barPosition) {
    if (playHead == nullptr) return;
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

void GeneratingBufferingProcessor::processAndRouteMidi(vector<PPMidiMessage>& midiMessageQueue, int blockSize, Synth* synth) {
    auto it = midiMessageQueue.begin();
    while (it != midiMessageQueue.end()) {
        if (it->ts < blockSize) {
            switch (it->type) {
            case PPMidiMessage::Type::On:
                synth->handleNoteOnOff(it->channel, it->note, it->value, true);
                break;
            case PPMidiMessage::Type::Off:
                synth->handleNoteOnOff(it->channel, it->note, it->value, false);
                break;
            default:
                assert(0);
                break;
            }
            it = midiMessageQueue.erase(it);
        }
        else {
            it->ts -= blockSize;
            it++;
        }
    }
}

void GeneratingBufferingProcessor::process(AudioSampleBuffer& buffer, vector<PPMidiMessage>& midiMessageQueue, float sampleRate, Synth* synth, AudioPlayHead* playHead) {
    const int blockSize = buffer.getNumSamples();
    const int internalBlockSize = Synth::internalBlockSize();

    assert(outputBufferSamples >= 0);
    assert(outputBufferSamples <= internalBlockSize);

    int bufferOffset = (internalBlockSize - outputBufferSamples) % internalBlockSize;

    // samples from last call
    int destinationBufferOffset = 0;
    if (bufferOffset > 0) {
        auto* l = buffer.getWritePointer(0);
        auto* r = buffer.getWritePointer(1);
        int i = 0;
        for (i = 0; i < outputBufferSamples && i < blockSize; ++i) {
            l[i] = outputBuffer[0][bufferOffset + i];
            r[i] = outputBuffer[1][bufferOffset + i];
            outputBuffer[0][bufferOffset + i] = 0.f;
            outputBuffer[1][bufferOffset + i] = 0.f;
        }
        destinationBufferOffset += i;
        outputBufferSamples -= i;
    }

    if (outputBufferSamples > 0) {
        assert(destinationBufferOffset == blockSize);
        return; // no processing until we've emptied the outputBuffer
    }

    assert(outputBufferSamples == 0);

    const int nominalBlockSize = blockSize - destinationBufferOffset;
    const int alignedBlockSize = internalBlockSize * (nominalBlockSize / internalBlockSize);

    assert(alignedBlockSize >= 0);
    assert(alignedBlockSize <= blockSize);

    int carryOverSamples = nominalBlockSize - alignedBlockSize;

    assert((destinationBufferOffset + alignedBlockSize + carryOverSamples) == blockSize);
    assert((destinationBufferOffset + alignedBlockSize) <= blockSize);
    assert(carryOverSamples >= 0);

    // samples, if any, that fits a multiple of Synth::internalBlockSize
    if (alignedBlockSize > 0) {
        // TODO; do in chunks of internalBlockSize!
        handlePlayHead(synth, playHead, alignedBlockSize, sampleRate, barPosition);
        processAndRouteMidi(midiMessageQueue, alignedBlockSize, synth);
        synth->update(buffer.getWritePointer(0, destinationBufferOffset), buffer.getWritePointer(1, destinationBufferOffset), alignedBlockSize, sampleRate);
        destinationBufferOffset += alignedBlockSize;
    }

    // if not all samples fit, calculate a new frame and store
    if (carryOverSamples > 0) {
        handlePlayHead(synth, playHead, internalBlockSize, sampleRate, barPosition);
        processAndRouteMidi(midiMessageQueue, internalBlockSize, synth);
        synth->update(outputBuffer[0], outputBuffer[1], internalBlockSize, sampleRate);
        auto* l = buffer.getWritePointer(0, destinationBufferOffset);
        auto* r = buffer.getWritePointer(1, destinationBufferOffset);
        int i = 0;
        for (i = 0; i < carryOverSamples; ++i) {
            l[i] = outputBuffer[0][i];
            r[i] = outputBuffer[1][i];
            outputBuffer[0][i] = 0.f;
            outputBuffer[1][i] = 0.f;
        }
        outputBufferSamples = internalBlockSize - carryOverSamples;
        destinationBufferOffset += carryOverSamples;
    }

    assert(destinationBufferOffset == blockSize);
    assert(outputBufferSamples >= 0);
    assert(outputBufferSamples <= internalBlockSize);

}

void InputBufferingProcessor::process(AudioSampleBuffer& buffer, float sampleRate, Effect* effect, AudioPlayHead* playHead) {
    // TODO, merge inputBuffer into scratchBuffer to reduce amount of copying back and forth

    // pre checks
    assert(buffer.getNumChannels() >= 2);

    if (scratchBuffer.getNumChannels() != buffer.getNumChannels() || scratchBuffer.getNumSamples() != 2 * buffer.getNumSamples()) {
        scratchBuffer.setSize(buffer.getNumChannels(), 2 * buffer.getNumSamples(), true, true, true);
    }

    const int blockSize = buffer.getNumSamples();
    const int internalBlockSize = Effect::internalBlockSize();

    // buffer input and process, bump to scratchBuffer
    int i = 0;
    for (i = 0; i < internalBlockSize - inputBufferSamples && i < blockSize; i++) {
        inputBuffer[0][inputBufferSamples + i] = buffer.getSample(0, i);
        inputBuffer[1][inputBufferSamples + i] = buffer.getSample(1, i);
    }
    inputBufferSamples += i;

    auto scratchBufferSize = 0;
    if (inputBufferSamples == internalBlockSize) {
        handlePlayHead(effect, playHead, internalBlockSize, sampleRate, barPosition);
        effect->update(inputBuffer[0], inputBuffer[1], internalBlockSize, sampleRate);
        for (int j = 0; j < internalBlockSize; j++) {
            scratchBuffer.setSample(0, j, inputBuffer[0][j]);
            scratchBuffer.setSample(1, j, inputBuffer[1][j]);
        }
        scratchBufferSize += internalBlockSize;
        inputBufferSamples -= internalBlockSize;

        auto alignedBlockSize = internalBlockSize * ((blockSize - i) / internalBlockSize);
        if (alignedBlockSize) {
            handlePlayHead(effect, playHead, alignedBlockSize, sampleRate, barPosition);
            effect->update(buffer.getWritePointer(0, i), buffer.getWritePointer(1, i), alignedBlockSize, sampleRate);
            scratchBuffer.copyFrom(0, scratchBufferSize, buffer.getReadPointer(0, i), alignedBlockSize);
            scratchBuffer.copyFrom(1, scratchBufferSize, buffer.getReadPointer(1, i), alignedBlockSize);
        }
        scratchBufferSize += alignedBlockSize;

        int toBuffer = blockSize - i - alignedBlockSize;
        for (int j = 0; j < toBuffer && inputBufferSamples + j < internalBlockSize; j++) {
            inputBuffer[0][inputBufferSamples + j] = buffer.getSample(0, i + alignedBlockSize + j);
            inputBuffer[1][inputBufferSamples + j] = buffer.getSample(1, i + alignedBlockSize + j);
        }
        inputBufferSamples += toBuffer;
    }

    // copy samples from outputBuffer and scratchBuffer, bump remainder to outputBuffer
    int toCopy = 0;
    if (outputBufferSamples + scratchBufferSize >= blockSize) {
        int o = 0;
        for (o = 0; o < outputBufferSamples && o < blockSize; o++) {
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
}

// ...

ParameterEditor::ParameterEditor()
{
    addAndMakeVisible(pageTabs);
}

ParameterEditor::~ParameterEditor() {
    for (auto* p : pages) delete p;
}

void ParameterEditor::resized()
{
    pageTabs.setBoundsRelative(0.f, 0.0f, 1.f, 1.0f);
    repaint();
}

void ParameterEditor::addKnob(ParameterKnob* knob) {
    const int knobsPerPage = Parameters::knobsPerBank*Parameters::banksPerPage;
    if (0 == (knobCtr % knobsPerPage)) {
        auto* p = new PPGrid;
        p->setColoumns(rowLayout);
        pages.push_back(p);
        pageTabs.addTab(String(knobCtr) + "-" + String(knobCtr + knobsPerPage-1), Colours::black, p, false);
    }
    pages.back()->addComponent(knob);
    knobCtr++;
}
