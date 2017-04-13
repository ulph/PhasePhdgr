#define _USE_MATH_DEFINES
#include <math.h>

#include "biquad.hpp"

Biquad::Biquad()
{
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("a1"));
    inputs.push_back(Pad("a2"));
    inputs.push_back(Pad("b0", 1.f));
    inputs.push_back(Pad("b1"));
    inputs.push_back(Pad("b2"));
    outputs.push_back(Pad("output"));
    init_state();
}

void Biquad::init_state()
{
  x1 = x2 = 0.f;
  y1 = y2 = 0.f;
}

/**
 * @brief Biquad::process
 * @param fs
 *
 * a0 is assumed to be equal to 1. If not, all coefficients should be normalized by a0
 */
void Biquad::process(uint32_t fs) {
  float a1, a2, b0, b1, b2;
  a1 = inputs[1].value;
  a2 = inputs[2].value;
  b0 = inputs[3].value;
  b1 = inputs[4].value;
  b2 = inputs[5].value;

  outputs[0].value = b0 * inputs[0].value + b1 * x1 + b2 * x2 - a1 * y1 - a2 *y2;
  x2 = x1;
  x1 = inputs[0].value;
  y2 = y1;
  y1 = outputs[0].value;
}

LowPass::LowPass()
{
  inputs.push_back(Pad("f0"));
  inputs.push_back(Pad("Q", 1.0));

  outputs.push_back(Pad("a1"));
  outputs.push_back(Pad("a2"));
  outputs.push_back(Pad("b0"));
  outputs.push_back(Pad("b1"));
  outputs.push_back(Pad("b2"));
}

/**
 * @brief LowPass::process
 * @param fs
 *
 * LPF:        H(s) = 1 / (s^2 + s/Q + 1)
 *        b0 =  (1 - cos(w0))/2
 *        b1 =   1 - cos(w0)
 *        b2 =  (1 - cos(w0))/2
 *        a0 =   1 + alpha
 *        a1 =  -2*cos(w0)
 *        a2 =   1 - alpha
 */
void LowPass::process(uint32_t fs)
{
  float w0 = 2 * M_PI * inputs[0].value / fs;
  float alpha = sinf(w0) / (2.0 * inputs[1].value);

  float a0 = 1. + alpha;

  outputs[0].value = -2.f * cosf(w0) / a0;
  outputs[1].value = (1.f - alpha) / a0;
  outputs[2].value = (1.f - cosf(w0)) / (2. * a0);
  outputs[3].value = 2.f * outputs[2].value;
  outputs[4].value = outputs[2].value;

}
