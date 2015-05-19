#include "Vector.h"

template <typename T>
Vector2<T> Vector2<T>::operator+(Vector2<T>& vector) const
{
  return Vector2<T>(x + vector.x, y + vector.y);
}

template <typename T>
Vector2<T> Vector2<T>::operator-(Vector2<T>& vector) const
{
  return Vector2<T>(x - vector.x, y - vector.y);
}

template <typename T>
Vector2<T> Vector2<T>::operator*(const float scalar) const 
{
  return Vector2<T>(x * scalar, y * scalar);
}

template <typename T>
Vector2<T> Vector2<T>::operator/(const float scalar) const 
{
  return Vector2<T>(x / scalar, y / scalar);
}

template <typename T>
void Vector2<T>::operator+=(const Vector2<T>& vector)
{
  x += vector.x;
  y += vector.y;
}

template <typename T>
void Vector2<T>::operator-=(const Vector2<T>& vector)
{
  x -= vector.x;
  y -= vector.y;
}

template <typename T>
void Vector2<T>::operator*=(const Vector2<T>& vector)
{
  x *= vector.x;
  y *= vector.y;
}

template <typename T>
void Vector2<T>::operator*=(const float scalar)
{
  x *= scalar;
  y *= scalar;
}

template <typename T>
bool Vector2<T>::operator==(const Vector2<T>& vector) const 
{
  return x == vector.x && y == vector.y;
}

template <typename T>
bool Vector2<T>::operator!=(const Vector2<T>& vector) const 
{
  return x != vector.x || y != vector.y;
}

template <typename T>
float Vector2<T>::getLength() const 
{
  return sqrt(pow(x, 2) + pow(y, 2));
}

template <typename T>
void Vector2<T>::normalize() 
{
  float length = getLength();
  x /= length;
  y /= length;
}

template <typename T>
Vector2<T> Vector2<T>::directionVector(float angle)
{

  float radAng = ((M_PI)/180.0f) * angle;
  return Vector2<T>(cos(radAng), -sin(radAng));
}


// Vector3

template <typename T>
Vector3<T> Vector3<T>::operator+(Vector3<T>& vector) const
{
  return Vector3<T>(x + vector.x, y + vector.y, z + vector.z);
}

template <typename T>
Vector3<T> Vector3<T>::operator-(const Vector3<T>& vector) const
{
  return Vector3<T>(x - vector.x, y - vector.y, z - vector.z);
}

template <typename T>
Vector3<T> Vector3<T>::operator*(const float scalar) const 
{
  return Vector3<T>(x * scalar, y * scalar, z * scalar);
}

template <typename T>
void Vector3<T>::operator+=(const Vector3<T>& vector)
{
  x += vector.x;
  y += vector.y;
  z += vector.z;
}

template <typename T>
void Vector3<T>::operator-=(const Vector3<T>& vector)
{
  x -= vector.x;
  y -= vector.y;
  z -= vector.z;
}

template <typename T>
void Vector3<T>::operator*=(const Vector3<T>& vector)
{
  x *= vector.x;
  y *= vector.y;
  z *= vector.z;
}

template <typename T>
void Vector3<T>::operator*=(const float scalar)
{
  x *= scalar;
  y *= scalar;
  z *= scalar;
}

template <typename T>
bool Vector3<T>::operator==(const Vector3<T>& vector) const 
{
  return x == vector.x && y == vector.y && z == vector.z;
}

template <typename T>
bool Vector3<T>::operator!=(const Vector3<T>& vector) const 
{
  return x != vector.x || y != vector.y || z != vector.z;
}

template <typename T>
float Vector3<T>::getLength() const 
{
  return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

template <typename T>
Vector3<T> Vector3<T>::interpolate(const Vector3<T>& v1, const Vector3<T>& v2, float t)
{
  return v1 + (v2 - v1) * t ;
}


template <typename T>
Vector2<T> normalize(const Vector2<T>& vector)
{
  Vector2<T> resultVector;
  float length = vector.getLength();
  resultVector = vector / length; 
  return resultVector;
}

template <typename T>
Vector2<T> Rect<T>::getCorner(const int cornerIndex) const
{
  Vector2<T> corner;
  switch(cornerIndex){
  case 0: corner = Vector2<T>(left, top);
    break;
  case 1: corner = Vector2<T>(left + width, top);
    break;
  case 2: corner = Vector2<T>(left + width, top + height);
    break;
  case 3: corner = Vector2<T>(left, top + height);
    break;
  };
  return corner;
}

template <typename T>
Vector2<T> Rect<T>::operator[](const int cornerIndex) const
{
  return getCorner(cornerIndex);
}

template <typename T>
void Rect<T>::operator+=(const Vector2<T>& delta)
{
  left += delta.x;
  top += delta.y;
}

template <typename T>
Rect<T> Rect<T>::operator+(const Vector2<T>& delta)
{
  Rect<T> result;
  result = *this;
  result.left += delta.x;
  result.top += delta.y; 

  return result;
}

template <typename T>
bool Rect<T>::doesContain(const Vector2<T>& point) const
{
  if(point.x >= left && point.x <= (left + width) &&
     point.y >= top && point.y <= (top + height)) return true;
  
  return false;
}

template <typename T>
bool Rect<T>::doesRectCollideWith(const Rect<T>& collisionRect) const
{
  
  
  return !(left > (collisionRect.left + collisionRect.width) ||
	   (left + width) < collisionRect.left ||
	   top > (collisionRect.top + collisionRect.height) ||
	   (top + height) < collisionRect.top);
}

template <typename T>
float Rect<T>::getMaxTime(const Vector2<T>& point, const Vector2f& deltaVector, const int wallIndex) const
{
  float result = 1000.0f;
  
  switch(wallIndex)
  {
  case 0:
    {
      // Upper Wall
      if(deltaVector.y != 0)
      {
	result = (top - point.y) / deltaVector.y;
	float newPositionX = point.x + result * deltaVector.x;
	if(newPositionX >= left && newPositionX <= left + width) return result;
	else return 1000.0f;
      }
      
    }break;
  case 1:
    {
      // Right Wall
      if(deltaVector.x != 0)
      {
	result = ((left + width) - point.x) / deltaVector.x;
	float newPositionY = point.y + result * deltaVector.y;
	
	if(newPositionY >= top && newPositionY <= top + height) return result;
	else return 1000.0f;
      }
    }break;
  case 2:
    {
      // Lower Wall
      if(deltaVector.y != 0)
      {
	result = ((top + height) - point.y) / deltaVector.y;
	float newPositionX = point.x + result * deltaVector.x;
	
	if(newPositionX > left && newPositionX < left + width) return result;
	else return 1000.0f;
      }
      
    }break;
  case 3:
    {
      // Left Wall
      if(deltaVector.x != 0)
      {
	result = (left - point.x) / deltaVector.x;
	float newPositionY = point.y + result * deltaVector.y;
	
	if(newPositionY >= top && newPositionY <= top + height) return result;
	else return 1000.0f;
      }
    }break;
  }

  return result;
}
