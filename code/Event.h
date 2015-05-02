#pragma once

#include "Types.h"
#include "Vector.h"
#include "EntityPosition.h"

#include <assert.h>
#include <string>
#include <unordered_map>

enum EVENT_ARGUMENT_TYPE{
  EAT_INT,
  EAT_FLOAT,
  EAT_STRING,
  EAT_VECTOR2I,
  EAT_VECTOR2F,
  EAT_VECTOR3I,
  EAT_WORLDPOSITION,
  EAT_ENTITYPOSITION,
  EAT_INVALID
};

class EventArgumentData{
 public:
 EventArgumentData() : eventArgumentType(EAT_INVALID) {};
  
  EventArgumentData(const EventArgumentData& eventArgumentData);
  ~EventArgumentData();
  
  EventArgumentData operator=(const EventArgumentData& eventArgumentData);
  EventArgumentData operator=(int32 value);
  
  EventArgumentData(const int32 value);
  EventArgumentData(const float value);
  EventArgumentData(const std::string value);
  
  EventArgumentData(const Vector2i value);
  EventArgumentData(const Vector2f value);
  EventArgumentData(const Vector3i value);
  
  EventArgumentData(const WorldPosition value);
  EventArgumentData(const EntityPosition value);
  
  int asInt() const;
  float asFloat() const;
  std::string asString() const;

  const Vector2i& asVector2i() const;
  const Vector2f& asVector2f() const;
  const Vector3i& asVector3i() const;
  
  const WorldPosition& asWorldPosition() const;
  const EntityPosition& asEntityPosition() const;
  
 private:
  EVENT_ARGUMENT_TYPE eventArgumentType;
  
  union {
    void* data;
    int32 intValue;
    float floatValue;
  };

  void deleteData();
  
};

// Argument set for given event
typedef std::unordered_map<std::string, EventArgumentData> EventArgumentDataMap;

// List Of Argument Sets
typedef std::list<EventArgumentDataMap> EventArgumentMapList;

// Each Event Has List Argument sets
typedef std::unordered_map<std::string, EventArgumentMapList> EventMapList;

// Event Name List
typedef std::list<std::string> EventNameList;

class EventOperator {
public:
  // Return Local Events To Send Should Be Cleared After Access
  EventMapList getQueuedEvents();

  // Returns string list of events that it's interestd in 
  virtual EventNameList getEntityEvents() { return EventNameList(); }
  
  void queueEvent(const std::string& eventName, const EventArgumentDataMap& eventArgumentMap);
  virtual void onEvent(const std::string& eventName, EventArgumentDataMap eventDataMap) {};
  
 private:
  EventMapList localEventMapList;
};
