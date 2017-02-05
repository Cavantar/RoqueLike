template <typename T>
Vec2<T> Rect<T>::getCorner(const int cornerIndex) const
{
  Vec2<T> corner;
  switch(cornerIndex){
  case 0: corner = Vec2<T>(left, top);
    break;
  case 1: corner = Vec2<T>(left + width, top);
    break;
  case 2: corner = Vec2<T>(left + width, top + height);
    break;
  case 3: corner = Vec2<T>(left, top + height);
    break;
  };
  return corner;
}

template <typename T>
Vec2<T> Rect<T>::operator[](const int cornerIndex) const
{
  return getCorner(cornerIndex);
}

template <typename T>
void Rect<T>::operator+=(const Vec2<T>& delta)
{
  left += delta.x;
  top += delta.y;
}

template <typename T>
Rect<T> Rect<T>::operator+(const Vec2<T>& delta)
{
  Rect<T> result;
  result = *this;
  result.left += delta.x;
  result.top += delta.y;

  return result;
}

template <typename T>
bool Rect<T>::doesContain(const Vec2<T>& point) const
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
float Rect<T>::getMaxTime(const Vec2<T>& point, const Vec2f& deltaVec, const int wallIndex) const
{
  float result = 1000.0f;

  switch(wallIndex)
  {
  case 0:
    {
      // Upper Wall
      if(deltaVec.y != 0)
      {
	result = (top - point.y) / deltaVec.y;
	float newPositionX = point.x + result * deltaVec.x;
	if(newPositionX >= left && newPositionX <= left + width) return result;
	else return 1000.0f;
      }

    }break;
  case 1:
    {
      // Right Wall
      if(deltaVec.x != 0)
      {
	result = ((left + width) - point.x) / deltaVec.x;
	float newPositionY = point.y + result * deltaVec.y;

	if(newPositionY >= top && newPositionY <= top + height) return result;
	else return 1000.0f;
      }
    }break;
  case 2:
    {
      // Lower Wall
      if(deltaVec.y != 0)
      {
	result = ((top + height) - point.y) / deltaVec.y;
	float newPositionX = point.x + result * deltaVec.x;

	if(newPositionX > left && newPositionX < left + width) return result;
	else return 1000.0f;
      }

    }break;
  case 3:
    {
      // Left Wall
      if(deltaVec.x != 0)
      {
	result = (left - point.x) / deltaVec.x;
	float newPositionY = point.y + result * deltaVec.y;

	if(newPositionY >= top && newPositionY <= top + height) return result;
	else return 1000.0f;
      }
    }break;
  }

  return result;
}
