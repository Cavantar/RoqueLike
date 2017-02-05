#pragma once

#include "Vector.h"

template <typename T>
class Rect{
public:
  T left;
  T top;
  T width;
  T height;

  Rect(T left = 0,T top = 0,  T width = 0, T height = 0) :
    left(left), top(top),  width(width), height(height) {}

  Vec2<T> getCorner(const int cornerIndex) const ;
  Vec2<T> operator[](const int cornerIndex) const ;

  void operator+=(const Vec2<T>& delta);
  Rect<T> operator+(const Vec2<T>& delta);

  bool doesContain(const Vec2<T>& point) const ;
  bool doesRectCollideWith(const Rect<T>& collisionRect) const;

  // Top Right Down Left
  float getMaxTime(const Vec2<T>& point, const Vec2f& deltaVec, const int wallIndex) const;
};

typedef Rect<int> IntRect;
typedef Rect<float> FloatRect;

#include "Rect.cpp"
