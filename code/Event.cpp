#include "Event.h"

EventArgumentData::EventArgumentData(const EventArgumentData& eventArgumentData)
{
  eventArgumentType = eventArgumentData.eventArgumentType;
  void* dataSrc = eventArgumentData.data;
  
  switch(eventArgumentType)
  {
  case EAT_INT :
  case EAT_FLOAT :
    data = dataSrc;
    break;
  case EAT_STRING :
    data = new std::string(*((std::string*)dataSrc ));
    break;
  case EAT_VECTOR2I :
    data = new Vector2i(*((Vector2i*)dataSrc ));
    break;
  case EAT_VECTOR2F :
    data = new Vector2f(*((Vector2f*)dataSrc ));
    break;
  case EAT_VECTOR3I :
    data = new Vector3f(*((Vector3f*)dataSrc ));
    break;
  case EAT_WORLDPOSITION :
    data = new WorldPosition(*((WorldPosition*)dataSrc));
    break;
  case EAT_ENTITYPOSITION :
    data = new EntityPosition(*((EntityPosition*)dataSrc ));
    break;
  defaut:
    assert(0);
  }
}
  
EventArgumentData::EventArgumentData(const int32 value)
{
  eventArgumentType = EAT_INT;
  intValue = value;
}

EventArgumentData::EventArgumentData(const float value)
{
  eventArgumentType = EAT_FLOAT;
  floatValue = value;
}

EventArgumentData::EventArgumentData(const std::string value)
{
  eventArgumentType = EAT_STRING;
  data = new std::string(value);
}

EventArgumentData::EventArgumentData(const Vector2i value)
{
  eventArgumentType = EAT_VECTOR2I;
  data = new Vector2i(value);
}

EventArgumentData::EventArgumentData(const Vector2f value)
{
  eventArgumentType = EAT_VECTOR2F;
  data = new Vector2f(value);
}

EventArgumentData::EventArgumentData(const Vector3i value)
{
  eventArgumentType = EAT_VECTOR3I;
  data = new Vector3i(value);
}

EventArgumentData::EventArgumentData(const WorldPosition value)
{
  eventArgumentType = EAT_WORLDPOSITION;
  data = new WorldPosition(value);
}

EventArgumentData::EventArgumentData(const EntityPosition value)
{
  eventArgumentType = EAT_ENTITYPOSITION;
  data = new EntityPosition(value);
}

EventArgumentData::~EventArgumentData()
{
  deleteData();
}

EventArgumentData EventArgumentData::operator=(const EventArgumentData& eventArgumentData)
{
  deleteData();
  
  eventArgumentType = eventArgumentData.eventArgumentType;
  void* dataSrc = eventArgumentData.data;
  
  switch(eventArgumentType)
  {
  case EAT_INT :
  case EAT_FLOAT :
  case EAT_INVALID :
    data = dataSrc;
    break;
  case EAT_STRING :
    data = new std::string(*((std::string*)dataSrc ));
    break;
  case EAT_VECTOR2I :
    data = new Vector2i(*((Vector2i*)dataSrc ));
    break;
  case EAT_VECTOR2F :
    data = new Vector2f(*((Vector2f*)dataSrc ));
    break;
  case EAT_VECTOR3I :
    data = new Vector3f(*((Vector3f*)dataSrc ));
    break;
  case EAT_WORLDPOSITION :
    data = new WorldPosition(*((WorldPosition*)dataSrc));
    break;
  case EAT_ENTITYPOSITION :
    data = new EntityPosition(*((EntityPosition*)dataSrc ));
    break;
  defaut:
    assert(0);
  }
  
  return *this;
}

EventArgumentData EventArgumentData::operator=(int32 value)
{
  deleteData();
  intValue = value;
  
  return *this;
}

int EventArgumentData::asInt() const
{
  assert(eventArgumentType == EAT_INT);
  return intValue;
}

float EventArgumentData::asFloat() const
{
  assert(eventArgumentType == EAT_FLOAT);
  return floatValue;
}

std::string EventArgumentData::asString() const
{
  assert(eventArgumentType == EAT_STRING);
  return *(std::string *)data;
}

const Vector2i& EventArgumentData::asVector2i() const
{
  assert(eventArgumentType == EAT_VECTOR2I);
  return *(Vector2i*)data;
}

const Vector2f& EventArgumentData::asVector2f() const
{
  assert(eventArgumentType == EAT_VECTOR2F);
  return *(Vector2f*)data;
}

const Vector3i& EventArgumentData::asVector3i() const
{
  assert(eventArgumentType == EAT_VECTOR3I);
  return *(Vector3i*)data;
}

const WorldPosition& EventArgumentData::asWorldPosition() const
{
  assert(eventArgumentType == EAT_WORLDPOSITION);
  return *(WorldPosition*)data;
}

const EntityPosition& EventArgumentData::asEntityPosition() const
{
  assert(eventArgumentType == EAT_ENTITYPOSITION);
  return *(EntityPosition*)data;
}

void EventArgumentData::deleteData()
{
  // If memory is allocated
  if(eventArgumentType != EAT_INT && eventArgumentType != EAT_FLOAT && eventArgumentType != EAT_INVALID)
  {
    switch(eventArgumentType)
    {
    case EAT_STRING :
      delete (std::string*)data;
      break;
    case EAT_VECTOR2I :
      delete (Vector2i*)data;
      break;
    case EAT_VECTOR2F :
      delete (Vector2f*)data;
      break;
    case EAT_VECTOR3I :
      delete (Vector3i*)data;
      break;
    case EAT_WORLDPOSITION :
      delete (WorldPosition*)data;
      break;
    case EAT_ENTITYPOSITION :
      delete (EntityPosition*)data;
      break;
    defaut:
      assert(0);
    }
  }
}

void EventOperator::queueEvent(const std::string& eventName, const EventArgumentDataMap& eventArgumentMap)
{
  localEventMapList[eventName].push_back(eventArgumentMap);
}

EventMapList EventOperator::getQueuedEvents()
{
  EventMapList objToReturn = localEventMapList;
  localEventMapList.clear();
  return objToReturn;
}
