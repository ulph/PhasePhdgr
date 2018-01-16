#pragma once

#include <math.h>

typedef float(*unary_float_op)(float);

#define MAKE_UNARY_OPERATOR_CLASS( CLASSNAME, OP ) \
class CLASSNAME : public ModuleCRTP< CLASSNAME > \
{ \
public: \
    CLASSNAME() { \
        inputs.push_back(Pad("in")); \
        outputs.push_back(Pad("out")); \
    } \
\
    virtual void process() override { \
        outputs[0].value = OP(inputs[0].value); \
    } \
\
    static Module* factory() { return new CLASSNAME(); } \
};

#define MAKE_NORMALIZED_UNARY_OPERATOR_CLASS( CLASSNAME, OP ) \
class CLASSNAME : public ModuleCRTP< CLASSNAME > \
{ \
public: \
    CLASSNAME() { \
        inputs.push_back(Pad("in")); \
        inputs.push_back(Pad("prescaler", 1.0f)); \
        outputs.push_back(Pad("out")); \
    } \
\
    virtual void process() override { \
        float s = inputs[1].value; \
        float f = 1.0f / OP(s); \
        outputs[0].value = OP(inputs[0].value * s) * f; \
    } \
\
    static Module* factory() { return new CLASSNAME(); } \
};

#define MAKE_STEREO_NORMALIZED_UNARY_OPERATOR_CLASS( CLASSNAME, OP ) \
class CLASSNAME : public ModuleCRTP< CLASSNAME > \
{ \
public: \
    CLASSNAME() { \
        inputs.push_back(Pad("left")); \
        inputs.push_back(Pad("right")); \
        inputs.push_back(Pad("prescaler", 1.0f)); \
        outputs.push_back(Pad("left")); \
        outputs.push_back(Pad("right")); \
    } \
\
    virtual void process() override { \
        float s = inputs[2].value; \
        float f = 1.0f / OP(s); \
        outputs[0].value = OP(inputs[0].value * s) * f; \
        outputs[1].value = OP(inputs[1].value * s) * f; \
    } \
\
    static Module* factory() { return new CLASSNAME(); } \
};

MAKE_UNARY_OPERATOR_CLASS( ArcSinH, asinhf )
MAKE_UNARY_OPERATOR_CLASS( TanH, tanhf )
MAKE_UNARY_OPERATOR_CLASS( Atan, atanf )

MAKE_NORMALIZED_UNARY_OPERATOR_CLASS( NormalizedArcSinH, asinhf )
MAKE_NORMALIZED_UNARY_OPERATOR_CLASS( NormalizedTanH, tanhf )
MAKE_NORMALIZED_UNARY_OPERATOR_CLASS( NormalizedAtan, atanf )

MAKE_STEREO_NORMALIZED_UNARY_OPERATOR_CLASS( StereoNormalizedArcSinH, asinhf )
MAKE_STEREO_NORMALIZED_UNARY_OPERATOR_CLASS( StereoNormalizedTanH, tanhf )
MAKE_STEREO_NORMALIZED_UNARY_OPERATOR_CLASS( StereoNormalizedAtan, atanf )

//

MAKE_UNARY_OPERATOR_CLASS( Abs, fabsf )