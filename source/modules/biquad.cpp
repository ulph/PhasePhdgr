#include "biquad.hpp"

Biquad::Biquad()
  : x1(0.f)
  , x2(0.f)
  , y1(0.f)
  , y2(0.f)
{
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("a1"));
    inputs.push_back(Pad("a2"));
    inputs.push_back(Pad("b0", 1.f));
    inputs.push_back(Pad("b1"));
    inputs.push_back(Pad("b2"));
    outputs.push_back(Pad("output"));
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
