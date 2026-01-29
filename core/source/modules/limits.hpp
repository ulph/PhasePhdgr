#pragma once

static inline float limitLow(float value, float low=0.0f) {
    return value < low ? low : value;
}

static inline float limit(float value, const float low=-1.f, const float high=1.f){
    return (value>high)?high:((value<low)?low:value);
}

static inline bool isHigh(float value){
    return value >= 0.5f;
}