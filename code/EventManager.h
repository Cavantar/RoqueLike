#pragma once

#include "Event.h"

#ifdef UNITY_BUILD
#include "Event.cpp"
#endif

typedef std::unordered_map<std::string, std::list<EventOperator*>> EventListenerList;
typedef std::list<EventOperator*> EventBroadcasterList;

class EventManager{
public:
  void registerListener(EventOperator* eventListener);
  void collectEvents();
  void dispatchEvents();
  
  void queueEvent(std::string eventName, const EventArgumentDataMap& eventArgumentDataMap);
  void reset();
private:
  EventMapList eventQueue;
  EventListenerList eventListenerList;
  EventBroadcasterList eventBroadcasterList;
  
  bool isSpecialEvent(std::string eventName) const ;
  void removeListener(EventOperator* eventListener);
  void handleSpecialEvent(const std::string& eventName, const EventArgumentMapList& eventArgumentMapList);
  
};
