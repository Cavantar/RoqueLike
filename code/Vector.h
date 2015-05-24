#pragma once
#include "types.h"
#include <unordered_map>

#define M_PI 3.14159265358979323846

enum CARDINAL_DIRECTION{
  CD_UP,
  CD_RIGHT,
  CD_DOWN,
  CD_LEFT
};

template <typename T>
class Vector2{
 public:
  T x;
  T y;
  
  Vector2<T>(const T x=0, const T y=0) : x(x), y(y) {}

  Vector2<T> operator+(Vector2<T>& vector) const ;
  Vector2<T> operator-(Vector2<T>& vector) const ;
  Vector2<T> operator*(const float scalar) const ;
  Vector2<T> operator/(const float scalar) const ;
  
  void operator+=(const Vector2<T>& vector);
  void operator-=(const Vector2<T>& vector);
  void operator*=(const Vector2<T>& vector);
  void operator*=(const float scalar);
  
  bool operator==(const Vector2<T>& vector) const ;
  bool operator!=(const Vector2<T>& vector) const ;

  float getLength() const ;
  void normalize();
  static Vector2<T> directionVector(float angle = rand()%360);
  static Vector2<T> cardinalDirection(CARDINAL_DIRECTION cardinalDirection);
  
  void showData() const;
};

typedef Vector2<int32> Vector2i;
typedef Vector2<float> Vector2f;

template <typename T>
class Vector3{
 public:
  T x;
  T y;
  T z;
  
  Vector3<T>(const T x=0, const T y=0, const T z=0) : x(x), y(y), z(z) {}
  
  Vector3<T> operator+(Vector3<T>& vector) const ;
  Vector3<T> operator-(const Vector3<T>& vector) const ;
  Vector3<T> operator*(const float scalar) const ;
  
  void operator+=(const Vector3<T>& vector);
  void operator-=(const Vector3<T>& vector);
  void operator*=(const Vector3<T>& vector);
  void operator*=(const float scalar);
  
  bool operator==(const Vector3<T>& vector) const ;
  bool operator!=(const Vector3<T>& vector) const ;
  
  float getLength() const ;
  static Vector3<T> interpolate(const Vector3<T>& v1, const Vector3<T>& v2, float t);
};

typedef Vector3<int32> Vector3i;
typedef Vector3<float> Vector3f;

template <typename T>
Vector2<T> normalize(const Vector2<T>& vector);

namespace std {
  
  template <> struct hash<Vector3i>
    {
      std::size_t operator()(const Vector3i& k) const
	{
	  using std::size_t;
	  using std::hash;
	  using std::string;

	  // Compute individual hash values for first,
	  // second and third and combine them using XOR
	  // and bit shifting:
      
	  return ((hash<int32>()(k.x)
		   ^ (hash<int32>()(k.y) << 1)) >> 1)
	    ^ (hash<int32>()(k.z) << 1);
	}
    };
}


template <typename T>
class Rect{
 public:
  T left;
  T top;
  T width;
  T height;
  
 Rect(T left = 0,T top = 0,  T width = 0, T height = 0) :
  left(left), top(top),  width(width), height(height) {}
  
  Vector2<T> getCorner(const int cornerIndex) const ;
  Vector2<T> operator[](const int cornerIndex) const ;
  
  void operator+=(const Vector2<T>& delta);
  Rect<T> operator+(const Vector2<T>& delta);

  bool doesContain(const Vector2<T>& point) const ;
  bool doesRectCollideWith(const Rect<T>& collisionRect) const;

  // Top Right Down Left 
  float getMaxTime(const Vector2<T>& point, const Vector2f& deltaVector, const int wallIndex) const;
};

typedef Rect<int> IntRect; 
typedef Rect<float> FloatRect; 

// Because Templates 
#ifndef UNITY_BUILD
#include "Vector.cpp"
#endif
