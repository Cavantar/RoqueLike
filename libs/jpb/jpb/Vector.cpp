// Definitions
template <typename T>
Vec2<T> Vec2<T>::operator+(Vec2<T>& vector) const
{
  return Vec2<T>(x + vector.x, y + vector.y);
}

template <typename T>
Vec2<T> Vec2<T>::operator-(const Vec2<T>& vector) const
{
  return Vec2<T>(x - vector.x, y - vector.y);
}

template <typename T>
Vec2<T> Vec2<T>::operator*(const float scalar) const
{
  return Vec2<T>(x * scalar, y * scalar);
}

template <typename T>
Vec2<T> Vec2<T>::operator/(const float scalar) const
{
  return Vec2<T>(x / scalar, y / scalar);
}

template <typename T>
void Vec2<T>::operator+=(const Vec2<T>& vector)
{
  x += vector.x;
  y += vector.y;
}

template <typename T>
void Vec2<T>::operator-=(const Vec2<T>& vector)
{
  x -= vector.x;
  y -= vector.y;
}

template <typename T>
void Vec2<T>::operator*=(const Vec2<T>& vector)
{
  x *= vector.x;
  y *= vector.y;
}

template <typename T>
void Vec2<T>::operator*=(const float scalar)
{
  x *= scalar;
  y *= scalar;
}

template <typename T>
bool Vec2<T>::operator==(const Vec2<T>& vector) const
{
  return x == vector.x && y == vector.y;
}

template <typename T>
bool Vec2<T>::operator!=(const Vec2<T>& vector) const
{
  return x != vector.x || y != vector.y;
}

template <typename T>
float Vec2<T>::getLength() const
{
  return sqrt(pow(x, 2) + pow(y, 2));
}

template <typename T>
void Vec2<T>::normalize()
{
  float length = getLength();
  x /= length;
  y /= length;
}

template <typename T>
Vec2<T> Vec2<T>::normalize(const Vec2<T>& vector)
{
  Vec2<T> temp = vector;
  temp.normalize();
  return temp;
}

template <typename T>
Vec2<T> Vec2<T>::directionVector(float angle)
{

  float radAng = ((M_PI)/180.0f) * angle;
  return Vec2<T>(cos(radAng), -sin(radAng));
}

template <typename T>
Vec2<T> Vec2<T>::cardinalDirection(CARDINAL_DIRECTION cardinalDirection)
{
  Vec2f result;

  switch(cardinalDirection)
  {
  case CD_UP:
    result = Vec2<T>(0, (T)-1);
    break;
  case CD_RIGHT:
    result = Vec2<T>((T)1, 0);
    break;
  case CD_DOWN:
    result = Vec2<T>(0, (T)1);
    break;
  case CD_LEFT:
    result = Vec2<T>((T)-1, 0);
    break;
  default:
    std::cout << "Incorrect direction \n";
  }

  return result;
}

template <typename T>
float Vec2<T>::dotProduct(const Vec2<T>& vector, const T x, const T y)
{
  return vector.x * x + vector.y * y;
}

template <typename T>
Vec2<float> Vec2<T>::lerp(const Vec2<T>& v1,const Vec2<T>& v2, const float t)
{
  return v1 + (v2 - v1) * t;
}

template <typename T>
void Vec2<T>::rotate(float angle)
{
  float radAng = (angle / 180.0f) * M_PI;

  Vec2f rotatedVector;
  rotatedVector.x = x * cos(radAng) - y * sin(radAng);
  rotatedVector.y = x * sin(radAng) + y * cos(radAng);

  *this = rotatedVector;
}

template <typename T>
float Vec2<T>::getXFor(real32 yValue, const Vec2<T>& startPosition) const
{
  // For what x y value will be 0
  // y = ax + b

  real32 a = y / x;
  real32 n_x = ((yValue - startPosition.y) / a) + startPosition.x;

  return n_x;
}

template <typename T>
void Vec2<T>::showData() const
{
  std::cout << "x: " << x << " y: " << y << std::endl;
}

// Vec3

template <typename T>
Vec3<T> Vec3<T>::operator+(Vec3<T>& vector) const
{
  return Vec3<T>(x + vector.x, y + vector.y, z + vector.z);
}

template <typename T>
Vec3<T> Vec3<T>::operator-(const Vec3<T>& vector) const
{
  return Vec3<T>(x - vector.x, y - vector.y, z - vector.z);
}

template <typename T>
Vec3<T> Vec3<T>::operator*(const Vec3<T>& vector) const
{
  Vec3<T> result = *this;
  result *= vector;
  return result;
}

template <typename T>
Vec3<T> Vec3<T>::operator*(const float scalar) const
{
  return Vec3<T>(x * scalar, y * scalar, z * scalar);
}


template <typename T>
Vec3<T> Vec3<T>::operator/(const float scalar) const
{
  return Vec3<T>(x / scalar, y / scalar, z / scalar);
}

template <typename T>
void Vec3<T>::operator+=(const Vec3<T>& vector)
{
  x += vector.x;
  y += vector.y;
  z += vector.z;
}

template <typename T>
void Vec3<T>::operator-=(const Vec3<T>& vector)
{
  x -= vector.x;
  y -= vector.y;
  z -= vector.z;
}

template <typename T>
void Vec3<T>::operator*=(const Vec3<T>& vector)
{
  x *= vector.x;
  y *= vector.y;
  z *= vector.z;
}

template <typename T>
void Vec3<T>::operator*=(const float scalar)
{
  x *= scalar;
  y *= scalar;
  z *= scalar;
}


template <typename T>
void Vec3<T>::operator/=(const float scalar)
{
  x /= scalar;
  y /= scalar;
  z /= scalar;
}

template <typename T>
bool Vec3<T>::operator==(const Vec3<T>& vector) const
{
  return x == vector.x && y == vector.y && z == vector.z;
}

template <typename T>
bool Vec3<T>::operator!=(const Vec3<T>& vector) const
{
  return x != vector.x || y != vector.y || z != vector.z;
}

template <typename T>
float Vec3<T>::getLength() const
{
  return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

template <typename T>
Vec2<T> Vec3<T>::toVec2() const
{
  return Vec2<T>(x, y);
}

template <typename T>
Vec3<real32> Vec3<T>::degToRad() const
{
  Vec3f result;
  real32 angRadValue = (M_PI/180.0f);

  result.x = angRadValue * x;
  result.y = angRadValue * y;
  result.z = angRadValue * z;

  return result;
}

template <typename T>
Vec3<T> Vec3<T>::normalize(const Vec3<T>& vector)
{
  Vec3<T> result = vector;
  float length = vector.getLength();
  result /= length;
  return result;
}

template <typename T>
real32 Vec3<T>::dotProduct(const Vec3<T>& v1, const Vec3<T>& v2)
{
  real32 result =  (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
  return result;
}

template <typename T>
Vec3<T> Vec3<T>::lerp(const Vec3<T>& v1, const Vec3<T>& v2, float t)
{
  return v1 + (v2 - v1) * t ;
}

template <typename T>
Vec3<float> Vec3<T>::cross(const Vec3<T>& v1, const Vec3<T>& v2)
{
  Vec3<float> result;

  result.x = (v1.y * v2.z) - (v1.z * v2.y);
  result.y = (v1.z * v2.x) - (v1.x * v2.z);
  result.z = (v1.x * v2.y) - (v1.y * v2.x);

  return result;
}

template <typename T>
void Vec3<T>::rotateAroundX(float radAngle)
{
  Vec3f rotatedPosition;

  // Rotation
  rotatedPosition.y = y * cos(radAngle) - z * sin(radAngle);
  rotatedPosition.z = y * sin(radAngle) + z * cos(radAngle);

  y = rotatedPosition.y;
  z = rotatedPosition.z;
}

template <typename T>
void Vec3<T>::rotateAroundY(float radAngle)
{
  Vec3f rotatedPosition;

  // Rotation
  rotatedPosition.x = x * cos(radAngle) - z * sin(radAngle);
  rotatedPosition.z = x * sin(radAngle) + z * cos(radAngle);

  x = rotatedPosition.x;
  z = rotatedPosition.z;
}

template <typename T>
void Vec3<T>::rotateAroundZ(float radAngle)
{
  Vec3f rotatedPosition;

  // Rotation
  rotatedPosition.x = x * cos(radAngle) - y * sin(radAngle);
  rotatedPosition.y = x * sin(radAngle) + y * cos(radAngle);

  x = rotatedPosition.x;
  y = rotatedPosition.y;
}

template <typename T>
void Vec3<T>::rotateAroundXDeg(float angle)
{
  Vec3f rotatedPosition;

  real32 radAngle = (M_PI / 180.0f) * angle;

  // Rotation
  rotatedPosition.y = y * cos(radAngle) - z * sin(radAngle);
  rotatedPosition.z = y * sin(radAngle) + z * cos(radAngle);

  y = rotatedPosition.y;
  z = rotatedPosition.z;
}

template <typename T>
void Vec3<T>::rotateAroundYDeg(float angle)
{
  Vec3f rotatedPosition;

  real32 radAngle = (M_PI / 180.0f) * angle;

  // Rotation
  rotatedPosition.x = x * cos(radAngle) - z * sin(radAngle);
  rotatedPosition.z = x * sin(radAngle) + z * cos(radAngle);

  x = rotatedPosition.x;
  z = rotatedPosition.z;
}

template <typename T>
void Vec3<T>::rotateAroundZDeg(float angle)
{
  Vec3f rotatedPosition;

  real32 radAngle = (M_PI / 180.0f) * angle;

  // Rotation
  rotatedPosition.x = x * cos(radAngle) - y * sin(radAngle);
  rotatedPosition.y = x * sin(radAngle) + y * cos(radAngle);

  x = rotatedPosition.x;
  y = rotatedPosition.y;
}


template <typename T>
void Vec3<T>::showData() const
{
  std::cout << "x: " << x << " y: " << y << " z: " << z << std::endl;
}

template <typename T>
Vec2<T> normalize(const Vec2<T>& vector)
{
  Vec2<T> resultVector;
  float length = vector.getLength();
  resultVector = vector / length;
  return resultVector;
}

template <typename T>
Vec4<T> Vec4<T>::operator*(const float scalar) const
{
  Vec4<T> result = *this;

  result.x *= scalar;
  result.y *= scalar;
  result.z *= scalar;
  result.w *= scalar;

  return result;
}

template <typename T>
Vec4<T> Vec4<T>::operator/(const float scalar) const
{
  float inverse = 1.0f/scalar;

  Vec4<T> result = (*this) * inverse;

  return result;
}

template <typename T>
void Vec4<T>::operator+=(const float value)
{
  x += value;
  y += value;
  z += value;
  w += value;
}

template <typename T>
void Vec4<T>::operator+=(const Vec4<T>& vector)
{
  x += vector.x;
  y += vector.y;
  z += vector.z;
  w += vector.w;
}

template <typename T>
void Vec4<T>::operator*=(const float scalar)
{
  x *= scalar;
  y *= scalar;
  z *= scalar;
  w *= scalar;
}

template <typename T>
T& Vec4<T>::operator[](const int index)
{
  return arr[index];
}


template <typename T>
void Vec4<T>::showData() const
{
  std::cout << "x: " << x << " y: " << y << " z: " << z << " w: " << w << std::endl;
}
