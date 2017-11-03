#pragma once

static inline float limit(float &value, const float low=-1.f, const float high=1.f){
    return (value>high)?high:((value<low)?low:value);
}