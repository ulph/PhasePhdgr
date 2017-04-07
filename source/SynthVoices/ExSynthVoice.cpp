#include "ExSynthVoice.h"

using namespace PhasePhckr;

void ExSynthVoice::reset(){
    // override defaults
    noiseEnv.setOnDecaySpeed(0.01f);
    env.setOffAttackSpeed(0.01);

};

void ExSynthVoice::update(float * buffer, int numSamples, float sampleRate){
    const MPEVoiceState &voice = mpe.getState();
    env.setSampleRate(sampleRate);
    noiseEnv.setSampleRate(sampleRate);

    for (int i = 0; i < numSamples; ++i) {
        // TODO, replace with connection diagram of ComponentsI

        env.put(0, voice.gate);
        noiseEnv.put(0, voice.gate);
        mpe.update();

        env.setOnBumpHeight(voice.strikeZ);
        env.setOffBumpHeight(voice.liftZ * 0.25);
        env.setSustainHeight(voice.pressZ);
        env.setOffDecaySpeed(0.15 + (1-voice.liftZ)*0.2);
        env.update();
        const float envV = env.get(0);

        noiseEnv.setOnBumpHeight(voice.strikeZ * 0.1);
        noiseEnv.setOffBumpHeight(voice.liftZ * 0.02);
        noiseEnv.setOffDecaySpeed(0.1f + (1-voice.liftZ) * 0.1f);
        noiseEnv.setSustainHeight(voice.pressZ * 0.01);
        noiseEnv.update();
        const float noiseEnvV = noiseEnv.get(0);

        float gain = envV;
        float oscMixB = 0.2;
        float oscMixF = voice.slideY + 0.2*envV;

        angle += voice.pitchHz / sampleRate * 2.0 * PI;
        float angleBoost = (1+10*gain);
        float angleFm = PI * atanf(angle * angleBoost) / atanf(PI * angleBoost);
        float sample = (float)(gain * ((1 - oscMixF) * sin(angleFm) + 0.5*(oscMixB+oscMixF) * sin(2 * angleFm) + sin(angle)));
        sample *= 0.5;

        float noise = noiseEnvV*rand()/RAND_MAX;
        noise *= noise;
        sample += noise;

        buffer[i] += 0.25*sample;

        if (angle > PI) angle -= 2 * PI;
    }
}
