#include "EventManager.h"

void EventManager::registerListener(EventOperator* eventListener)
{
  
  const EventNameList eventNameList = eventListener->getEntityEvents();
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

void EventManager::dispatchEvents()
{

  // For Each Event
  for(auto eventIt = eventQueue.begin(); eventIt != eventQueue.end(); eventIt++)
  {
    std::string eventName = eventIt->first;

    if(isSpecialEvent(eventName))
    {
      handleSpecialEvent(eventName, eventIt->second);
    }
    else
    {

      // Each Of The Listener Objects Receives, Each of the events data instances 
      std::list<EventOperator*> interestedObjects = eventListenerList[eventName];
    
      for(auto eventDataIt = eventIt->second.begin(); eventDataIt != eventIt->second.end(); eventDataIt++)
      {
	for(auto operatorIt = interestedObjects.begin(); operatorIt != interestedObjects.end(); operatorIt++)
	{
	  (*operatorIt)->onEvent(eventName, *eventDataIt);
	}
      }
    }
  }
  
  eventQueue.clear();
}

void EventManager::queueEvent(std::string eventName, const EventArgumentDataMap& eventArgumentDataMap)
{
  eventQueue[eventName].push_back(eventArgumentDataMap);
}

void EventManager::reset()
{
  eventQueue.clear();
  eventListenerList.clear();
  eventBroadcasterList.clear();
}

bool EventManager::isSpecialEvent(std::string eventName) const
{
  if(eventName == "EntityRemoved") return true;
  return false;
}

void EventManager::removeListener(EventOperator* eventListener)
{
  // Iteratng over events
  for(auto i = eventListenerList.begin(); i != eventListenerList.end(); i++)
  {
    // Iterating over listeners of those events
    for(auto listener = i->second.begin(); listener != i->second.end(); listener++)
    {
      if(*listener == eventListener)
      {
	// Removing listener if the pointer is the same
	i->second.erase(listener);
	break;
      }
    }
  }

  for(auto i = eventBroadcasterList.begin(); i != eventBroadcasterList.end(); i++)
  {
    if(*i == eventListener)
    {
      eventBroadcasterList.erase(i);
      break;
    }
  }
  
}

void EventManager::handleSpecialEvent(const std::string& eventName,
				      const EventArgumentMapList& eventArgumentMapList)
{
  for(auto i = eventArgumentMapList.begin(); i != eventArgumentMapList.end(); i++)
  {
    if(eventName == "EntityRemoved")
    {
      //std::cout << "RemovingEntities\n";
      const EventArgumentDataMap& eventArgumentDataMap = *i;

      EventOperator* eventOperator = (EventOperator*)eventArgumentDataMap.at("pointer").asPointer();
      removeListener(eventOperator);
    }
    
  }
}

