#pragma once

#include "PhasePhckrPluginCommon.h"

void proccessHandleCommonMidi(Effect* effect) {

}

void handlePlayHead(Effect* effect, AudioPlayHead* playHead, const int blockSize, const float sampleRate, float& barPosition) {
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
