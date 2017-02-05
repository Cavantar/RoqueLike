
#include "MiscFunctions.h"

float interp(float valueStart, float valueEnd, float t)
{
  return valueStart + ((valueEnd - valueStart) * t);
}

float dotProduct(const Vec2f& v1, const Vec2f& v2)
{
  return (v1.x * v2.x) + (v1.y * v2.y);
}

float dotProduct(const Vec2f& v1, float v2x, float v2y)
{
  return (v1.x * v2x) + (v1.y * v2y);
}

