#pragma once
#include <functional>

#if 0

#define _USE_MATH_DEFINES
#include <math.h>

#else

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif

// This thing is only temporary for debug stuff
#include <iostream>
#include "types.h"

enum CARDINAL_DIRECTION{
  CD_UP,
  CD_RIGHT,
  CD_DOWN,
  CD_LEFT
};

template <typename T>
class Vec2{
public:
  T x;
  T y;

  Vec2<T>(const T x=0, const T y=0) : x(x), y(y) {}

  Vec2<T> operator+(Vec2<T>& vector) const ;
  Vec2<T> operator-(const Vec2<T>& vector) const ;
  Vec2<T> operator*(const float scalar) const ;
  Vec2<T> operator/(const float scalar) const ;

  void operator+=(const Vec2<T>& vector);
  void operator-=(const Vec2<T>& vector);
  void operator*=(const Vec2<T>& vector);
  void operator*=(const float scalar);

  bool operator==(const Vec2<T>& vector) const ;
  bool operator!=(const Vec2<T>& vector) const ;

  float getLength() const ;
  void normalize();

  static Vec2<T> normalize(const Vec2<T>& vector);
  static Vec2<T> directionVector(float angle = rand()%360);
  static Vec2<T> cardinalDirection(CARDINAL_DIRECTION cardinalDirection);

  static float dotProduct(const Vec2<T>& vector, const T x, const T y);
  static Vec2<float> lerp(const Vec2<T>& v1,const Vec2<T>& v2, const float t);

  void rotate(float angle);

  // Returns X for lineEquation defined by the vector, starting at startPosition
  float getXFor(real32 yValue, const Vec2<T>& startPosition) const ;
  void showData() const;
};

typedef Vec2<int32> Vec2i;
typedef Vec2<uint32> Vec2u;
typedef Vec2<float> Vec2f;

template <typename T>
class Vec3{
public:
  T x;
  T y;
  T z;

  Vec3<T>(const T x=0, const T y=0, const T z=0) : x(x), y(y), z(z) {}

  Vec3<T> operator+(Vec3<T>& vector) const ;
  Vec3<T> operator-(const Vec3<T>& vector) const ;
  Vec3<T> operator-() const
  {
    Vec3<T> result = *this;
    result *= -1.0f;
    return result;
  }
  Vec3<T> operator*(const Vec3<T>& vector) const ;
  Vec3<T> operator*(const float scalar) const ;
  Vec3<T> operator/(const float scalar) const ;

  void operator+=(const Vec3<T>& vector);
  void operator-=(const Vec3<T>& vector);
  void operator*=(const Vec3<T>& vector);
  void operator*=(const float scalar);
  void operator/=(const float scalar);

  bool operator==(const Vec3<T>& vector) const ;
  bool operator!=(const Vec3<T>& vector) const ;

  float getLength() const ;
  Vec2<T> toVec2() const ;
  Vec3<real32> degToRad() const ;

  static Vec3<T> normalize(const Vec3<T>& vector);

  static float dotProduct(const Vec3<T>& v1, const Vec3<T>& v2);
  static Vec3<T> lerp(const Vec3<T>& v1, const Vec3<T>& v2, float t);
  static Vec3<real32> cross(const Vec3<T>& v1, const Vec3<T>& v2);

  static Vec3<T> rotateAround(const Vec3<T> src, real32 angle, Vec3& orbital)
  {
    Mat3 rotMat = Mat3::createRotationMatrix(angle, orbital);
    return rotMat * src;
  }

  void rotateAroundX(float radAngle);
  void rotateAroundY(float radAngle);
  void rotateAroundZ(float radAngle);

  void rotateAroundXDeg(float angle);
  void rotateAroundYDeg(float angle);
  void rotateAroundZDeg(float angle);

  void showData() const;
};

typedef Vec3<int32> Vec3i;
typedef Vec3<uint32> Vec3u;
typedef Vec3<real32> Vec3f;

template <typename T>
Vec2<T> normalize(const Vec2<T>& vector);

namespace std {
  template <> struct hash<Vec3i>
  {
    std::size_t operator()(const Vec3i& k) const
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
class Vec4 {
public:
  union{
    struct{
      T x;
      T y;
      T z;
      T w;
    };
    T arr[4];
  };

  Vec4<T>(const T x=0, const T y=0, const T z=0, const T w=0) : x(x), y(y), z(z), w(w) {}

  Vec4<T> operator*(const float scalar) const ;
  Vec4<T> operator/(const float scalar) const ;
  void operator+=(const float value) ;

  void operator+=(const Vec4<T>& vector) ;
  void operator*=(const float scalar) ;

  T& operator[](const int index) ;
  void showData() const;
};

typedef Vec4<int32> Vec4i;
typedef Vec4<uint32> Vec4u;
typedef Vec4<real32> Vec4f;

class Mat3 {
public:
  Mat3()
  {
    m[0] = Vec3f(1.0f, 0, 0);
    m[1] = Vec3f(0, 1.0f, 0);
    m[2] = Vec3f(0, 0, 1.0f);
  }
  Vec3f m[3];

  Vec3f operator*(const Vec3f& vector)
  {
    Vec3f result;
    result.x = Vec3f::dotProduct(m[0], vector);
    result.y = Vec3f::dotProduct(m[1], vector);
    result.z = Vec3f::dotProduct(m[2], vector);
    return result;
  }

  // Angle in degrees
  static Mat3 createRotationMatrix(real32 angle, Vec3f axis)
  {
    Vec3f normAxis = Vec3f::normalize(axis);
    Vec3f normAxisSqr = normAxis * normAxis;

    real32 angleRad = (M_PI / 180.0f) * angle;
    real32 cost = cos(angleRad);
    real32 sint = sin(angleRad);

    Mat3 result;

    result.m[0] = Vec3f(cost + normAxisSqr.x * (1-cost),
			normAxis.x * normAxis.y * (1-cost) - (normAxis.z * sint),
			normAxis.x * normAxis.z * (1-cost) + (normAxis.y * sint));

    result.m[1] = Vec3f(normAxis.y * normAxis.x * (1-cost) + (normAxis.z * sint),
			cost + normAxisSqr.y * (1-cost),
			normAxis.y * normAxis.z * (1-cost) - (normAxis.x * sint));

    result.m[2] = Vec3f(normAxis.z * normAxis.x * (1-cost) - (normAxis.y * sint),
			normAxis.z * normAxis.y * (1-cost) + (normAxis.x * sint),
			cost + normAxisSqr.z*(1-cost));

    return result;
  }



};

class Mat4 {
public:
  Mat4()
  {
    m[0] = Vec4f(1.0f, 0, 0, 0);
    m[1] = Vec4f(0, 1.0f, 0, 0);
    m[2] = Vec4f(0, 0, 1.0f, 0);
    m[3] = Vec4f(0, 0, 0, 1.0f);
  }

  Vec4f m[4];
  void transpose()
  {
    for(int y = 0; y < 4; y++)
    {
      for(int x = 0; x < 4; x++)
      {
	if (x == y) break;

	real32 val1 = m[y][x];
	real32 val2 = m[x][y];
	m[y][x] = val2;
	m[x][y] = val1;
      }
    }
  }

  Vec4f& operator[](const int index)
  {
    return m[index];
  }

  void showData()
  {
    std:: cout << "Mat4\n";
    std:: cout << "------------------\n";
    for(int i = 0; i < 4; i++)
    {
      Vec4f& row = m[i];

      std::cout << "[" << i << "]";
      for(int x = 0; x < 4; x++)
      {
	std::cout << " " << row[x];
      }
      std::cout << std::endl;
    }
    std:: cout << "------------------\n";
  }

  static Mat4 transpose(Mat4& src)
  {
    Mat4 result;
    for(int y = 0; y < 4; y++)
    {
      for(int x = 0; x < 4; x++)
      {
	result.m[y][x] = src.m[x][y];
	result.m[x][y] = src.m[y][x];
      }
    }
    return result;
  }

  // fov in degrees
  static Mat4 createPerspectiveMatrix(real32 fov, real32 aspectRatio, real32 nearZ, real32 farZ)
  {
    Mat4 perspectiveMatrix;
    real32 fovRad = (M_PI / 180.0f) * fov;
    real32 f = 1.0f / tan(fov / 2.0f);

    perspectiveMatrix[0] = Vec4f(f / aspectRatio, 0, 0, 0);
    perspectiveMatrix[1] = Vec4f(0, f, 0, 0);
    perspectiveMatrix[2] = Vec4f(0, 0, (farZ + nearZ) / (nearZ - farZ), (2*farZ *nearZ)/(nearZ - farZ));
    perspectiveMatrix[3] = Vec4f(0, 0, -1, 0);

    // newPerspectiveMatrix = glm::transpose(newPerspectiveMatrix);

    return perspectiveMatrix;
  }

  static Mat4 createTranslationMatrix(Vec3f translation)
  {
    Mat4 result;

    result[3] = Vec4f(translation.x, translation.y, translation.z, 1.0f);
    return result;
  }



};

#include "Vector.cpp"
