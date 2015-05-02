#include "EventManager.h"

void EventManager::registerListenerEvents(EventOperator* eventListener, EventNameList eventNameList)
{
  for(auto i = eventNameList.begin(); i != eventNameList.end(); i++)
  {
    eventListenerList[*i].push_back(eventListener);
  }

  // Adding Unique Listeners For Future Event Collection
  if(std::find(std::begin(eventBroadcasterList), std::end(eventBroadcasterList), eventListener) ==
     std::end(eventBroadcasterList))
  {
    eventBroadcasterList.push_back(eventListener);
  }
  
}

void EventManager::dispatchEvents()
{
  for(auto eventIt = eventQueue.begin(); eventIt != eventQueue.end(); eventIt++)
  {
    std::string eventName = eventIt->first;
    std::list<EventOperator*> interestedObjects = eventListenerList[eventName];
    
    for(auto eventDataIt = eventIt->second.begin(); eventDataIt != eventIt->second.end(); eventDataIt++)
    {
      for(auto operatorIt = interestedObjects.begin(); operatorIt != interestedObjects.end(); operatorIt++)
      {
	(*operatorIt)->onEvent(eventName, *eventDataIt);
      }
    }
  }
  
  eventQueue.clear();
}

void EventManager::queueEvent(std::string eventName, const EventArgumentDataMap& eventArgumentDataMap)
{
  eventQueue[eventName].push_back(eventArgumentDataMap);
}

void EventManager::collectEvents()
{
  // Getting Events from each of registered broadcasters 
  for(auto i = eventBroadcasterList.begin(); i != eventBroadcasterList.end(); i++)
  {
    EventMapList tempMapList = (*i)->getQueuedEvents();
    
    // Iterating over each of the events
    for(auto event = tempMapList.begin(); event != tempMapList.end(); event++)
    {
      const std::string& eventName = event->first;
      const EventArgumentMapList& argumentMapList = event->second;

      // Adding Each Of Argument Map To Given Event
      for(auto argumentMap = argumentMapList.begin(); argumentMap != argumentMapList.end(); argumentMap++)
      {
	eventQueue[eventName].push_back(*argumentMap);
      }
    }
  }
}

