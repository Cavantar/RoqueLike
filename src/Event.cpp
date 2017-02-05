#include "Event.h"

EventArgumentData::EventArgumentData(const EventArgumentData& eventArgumentData)
{
  eventArgumentType = eventArgumentData.eventArgumentType;
  void* dataSrc = eventArgumentData.data;
  
  switch(eventArgumentType)
  {
  case EAT_INT :
  case EAT_FLOAT :
  case EAT_POINTER :
    data = dataSrc;
    break;
  case EAT_STRING :
    data = new std::string(*((std::string*)dataSrc ));
    break;
  case EAT_VECTOR2I :
    data = new Vec2i(*((Vec2i*)dataSrc ));
    break;
  case EAT_VECTOR2F :
    data = new Vec2f(*((Vec2f*)dataSrc ));
    break;
  case EAT_VECTOR3I :
    data = new Vec3f(*((Vec3f*)dataSrc ));
    break;
  case EAT_WORLDPOSITION :
    data = new WorldPosition(*((WorldPosition*)dataSrc));
    break;
  case EAT_ENTITYPOSITION :
    data = new EntityPosition(*((EntityPosition*)dataSrc ));
    break;
  default:
    assert(0);
  }
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
    data = new Vec2i(*((Vec2i*)dataSrc ));
    break;
  case EAT_VECTOR2F :
    data = new Vec2f(*((Vec2f*)dataSrc ));
    break;
  case EAT_VECTOR3I :
    data = new Vec3f(*((Vec3f*)dataSrc ));
    break;
  case EAT_WORLDPOSITION :
    data = new WorldPosition(*((WorldPosition*)dataSrc));
    break;
  case EAT_ENTITYPOSITION :
    data = new EntityPosition(*((EntityPosition*)dataSrc ));
    break;
  default:
    assert(0);
  }
  
  return *this;
}

EventArgumentData EventArgumentData::operator=(int32 value)
{
  deleteData();
  
  eventArgumentType = EAT_INT;
  intValue = value;
  
  return *this;
}

EventArgumentData EventArgumentData::operator=(const float value)
{
  deleteData();
  
  eventArgumentType = EAT_FLOAT;
  floatValue = value;
  
  return *this;
}

EventArgumentData EventArgumentData::operator=(void * value)
{
  deleteData();
  
  eventArgumentType = EAT_POINTER;
  data = value;

  return *this;
}

EventArgumentData EventArgumentData::operator=(const std::string value)
{
  deleteData();
  
  eventArgumentType = EAT_STRING;
  data = new std::string(value);
  
  return *this;
}
  
EventArgumentData EventArgumentData::operator=(const Vec2i value)
{
  deleteData();
  
  eventArgumentType = EAT_VECTOR2I;
  data = new Vec2i(value);
  
  return *this;
}
EventArgumentData EventArgumentData::operator=(const Vec2f value)
{
  deleteData();
  
  eventArgumentType = EAT_VECTOR2F;
  data = new Vec2f(value);
  
  return *this;
}

EventArgumentData EventArgumentData::operator=(const Vec3i value)
{
  deleteData();
  
  eventArgumentType = EAT_VECTOR3I;
  data = new Vec3i(value);
  
  return *this;
}
  
EventArgumentData EventArgumentData::operator=(const WorldPosition value)
{
  deleteData();
  
  eventArgumentType = EAT_WORLDPOSITION;
  data = new WorldPosition(value);
  
  return *this;
}

EventArgumentData EventArgumentData::operator=(const EntityPosition value)
{
  deleteData();
  
  eventArgumentType = EAT_ENTITYPOSITION;
  data = new EntityPosition(value);
  
  return *this;
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

EventArgumentData::EventArgumentData(void* value)
{
  eventArgumentType = EAT_POINTER;
  data = value;
}

EventArgumentData::EventArgumentData(const std::string value)
{
  eventArgumentType = EAT_STRING;
  data = new std::string(value);
}

EventArgumentData::EventArgumentData(const Vec2i value)
{
  eventArgumentType = EAT_VECTOR2I;
  data = new Vec2i(value);
}

EventArgumentData::EventArgumentData(const Vec2f value)
{
  eventArgumentType = EAT_VECTOR2F;
  data = new Vec2f(value);
}

EventArgumentData::EventArgumentData(const Vec3i value)
{
  eventArgumentType = EAT_VECTOR3I;
  data = new Vec3i(value);
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

void* EventArgumentData::asPointer() const
{
  assert(eventArgumentType == EAT_POINTER);
  return data;
}

std::string EventArgumentData::asString() const
{
  assert(eventArgumentType == EAT_STRING);
  return *(std::string *)data;
}

const Vec2i& EventArgumentData::asVec2i() const
{
  assert(eventArgumentType == EAT_VECTOR2I);
  return *(Vec2i*)data;
}

const Vec2f& EventArgumentData::asVec2f() const
{
  assert(eventArgumentType == EAT_VECTOR2F);
  return *(Vec2f*)data;
}

const Vec3i& EventArgumentData::asVec3i() const
{
  assert(eventArgumentType == EAT_VECTOR3I);
  return *(Vec3i*)data;
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
  if(eventArgumentType != EAT_INT && eventArgumentType != EAT_FLOAT &&
     eventArgumentType != EAT_INVALID && eventArgumentType != EAT_POINTER)
  {
    switch(eventArgumentType)
    {
    case EAT_STRING :
      delete (std::string*)data;
      break;
    case EAT_VECTOR2I :
      delete (Vec2i*)data;
      break;
    case EAT_VECTOR2F :
      delete (Vec2f*)data;
      break;
    case EAT_VECTOR3I :
      delete (Vec3i*)data;
      break;
    case EAT_WORLDPOSITION :
      delete (WorldPosition*)data;
      break;
    case EAT_ENTITYPOSITION :
      delete (EntityPosition*)data;
      break;
    default:
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
