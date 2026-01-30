#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

#include <assert.h>

static inline double sinc(double x) {
    return x!=0 ? sin(x)/x : 1;
}

static inline double nsinc(double x) {
    return x!=0 ? sin(M_PI*x)/(M_PI*x) : 1;
}

const int c_numFractions = 1000; // TODO make templated arg

static inline int makeIndex(int N, int n, int i) {
    return i*N + n;
}

static inline double hamming(double i, double M) {
    return 0.54 + 0.46*cos((2.0*M_PI*i) / M);
}

static inline double blackman(double i, double M) {
    return 0.42 + 0.5*cos((2.0*M_PI*i) / M) + 0.08*cos((4.0*M_PI*i) / M);
}

template <int N>
struct FractionalSincTable
{
protected:
    double *coeffs;

public:
    explicit FractionalSincTable()
        : coeffs(new double[c_numFractions*N])
    {
        for (auto i = 0; i < c_numFractions; ++i)
        {
            auto frac = (double)i*1.0 / (double)(c_numFractions);
            for (auto n = 0; n < N; ++n)
            {
                auto ni = makeIndex(N, n, i);
                auto arg = ((double)n - frac - ((double)N - 1.0) / 2.0);
                auto w = blackman(arg, (double)N - 1.0);
                assert(std::isfinite(w));
                coeffs[ni] = (double)w*sinc(arg * M_PI);
                assert(std::isfinite(coeffs[ni]));
            }
        }
    };

    virtual ~FractionalSincTable() {
        delete[] coeffs;
    };

    int getCoefficientTablePointer(const double fraction, double** destinationBuffer, const int destinationBufferSize) const {
        if (destinationBufferSize < N)
        {
            assert(0);
            return 0;
        }
        if (fraction > 1.0 || fraction < 0.0) {
            assert(0);
            return 0;
        }

        const double softIdx = fraction * (double)c_numFractions;
        assert(softIdx < c_numFractions);
        const int i = (int)softIdx;
        assert(i < c_numFractions);
        
        int ni_0 = makeIndex(N, 0, i);
        *destinationBuffer = &coeffs[ni_0];

        return N;
    }

};

template <int N>
const FractionalSincTable<N> & getFractionalSincTable();
