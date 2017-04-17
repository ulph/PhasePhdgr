#pragma once

#include <math.h>

static inline float sincf(float x){
    return x!=0 ? sinf(x)/x : 1;
}

template<int N_, int numFractions_>
struct FractionalSincTable
{
    static_assert( (N_ % 2) != 0); // N must be odd
    FractionalSincTable()
        : N(N_)
        , numFractions(numFractions_)
        , coeffs()
    {
        for (auto i = 0; i < numFractions; ++i)
        {
            float frac = (float)i*1.0f/(float(numFractions));
            float M = 0;
            for (auto n = 0; n < N; ++n)
            {
                float arg = (float)n-frac-((float)N-1.f)/2.f;
                float hamming = 0.54f+0.46f*cosf( (2.f*M_PI*arg) / (float)(N-1) );
                coeffs[i][n] = hamming*sincf(arg);
                M += coeffs[i][n];
            }
            // silly normalization
            for (auto n = 0; n < N; ++n)
            {
                coeffs[i][n] /= M;
            }
        }
    }
    const int N;
    const int numFractions;
    float coeffs[numFractions_][N_];
};
const auto g_fracSincTable = FractionalSincTable<5, 1000>();
