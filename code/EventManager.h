#pragma once

#include "Event.h"

#ifdef UNITY_BUILD
#include "Event.cpp"
#endif

typedef std::unordered_map<std::string, std::list<EventOperator*>> EventListenerList;
typedef std::list<EventOperator*> EventBroadcasterList;

class EventManager{
public:
  void registerListenerEvents(EventOperator* eventListener, EventNameList eventNameList);
  void collectEvents();
  void dispatchEvents();
  
  void queueEvent(std::string eventName, const EventArgumentDataMap& eventArgumentDataMap);
private:
  EventMapList eventQueue;
  EventListenerList eventListenerList;
  EventBroadcasterList eventBroadcasterList;
};
