
#include "Profiler.h"

void
ProfilerEntry::start(float currentTime)
{
  sumTimeFrame = 0;
  numbOfFrameCalls = 0;
  
  startTime = currentTime;
}

void
ProfilerEntry::end(float currentTime)
{
  lastTime = currentTime - startTime;
  sumTimeFrame += lastTime;
  numbOfFrameCalls++;
}

void
ProfilerEntry::endFrame()
{
  avgTimeFrame = sumTimeFrame / numbOfFrameCalls;
}

void
Profiler::start(std::string region)
{
  profilerEntries[region].start(getCurrentTime());
}

void
Profiler::end(std::string region)
{
  profilerEntries[region].end(getCurrentTime());
}

void
Profiler::showData() const
{
  float totalTime = profilerEntries.at("Game").lastTime;
  
  for(auto i = profilerEntries.begin(); i != profilerEntries.end(); i++)
  {
    float percentageUse = i->second.lastTime / totalTime;
    
    std::cout << i->first << ":" ;
    if(i->second.numbOfFrameCalls == 1)
    {
      std::cout << "lastTime: " << i->second.lastTime * 1000 * 1000 << " \t";
    }
    else
    {
      std::cout << "sumTime: " << i->second.sumTimeFrame * 1000 * 1000 << " \t";
    }
    std:: cout << "Perc:" << percentageUse * 100 << " \t";

    if(i->second.numbOfFrameCalls != 1)
    {
      std:: cout << "numbOfIterations:" << i->second.numbOfFrameCalls << " \t";
    }
    
    if(i->second.numbOfFrameCalls != 1)
    {
      float percentageUsePerIteration = i->second.avgTimeFrame / totalTime;
      std::cout << "Perc2: " << percentageUsePerIteration * 100 << " \t";
    }
    
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void
Profiler::endFrame()
{
  for(auto i = profilerEntries.begin(); i != profilerEntries.end(); i++)
  {
    i->second.endFrame();
  }
  profilerEntries["Game"].end(getCurrentTime());
}

void
SfmlProfiler::startFrame()
{
  clock.restart();
  start("Game");
}

float
SfmlProfiler::getCurrentTime()
{
  return clock.getElapsedTime().asSeconds();
}
