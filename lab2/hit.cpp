#include <cmath>
#include <iostream>
#include "hit.h"

const float a = 2;

const float b = 8 * std::sqrt(2);

bool hit_test(float x, float y, float z) {
  return b * b * (y * y + z * z) <= x * x * x * (2 * a - x);
}

const float* get_axis_range() {
  float mx = (3 * std::sqrt(3) * a * a) / (4 * b);
  static float arr[6]{0, 2 * a, -mx, mx, -mx, mx};

  return arr;
}
