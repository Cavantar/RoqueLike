#include <windows.h>
#include <iomanip>
#include <iostream>
#include "Profiler.h"
#include "Types.h"

void
ProfilerEntry::start(double currentTime, uint64 cycleCount)
{
  startCount = cycleCount;
  startTime = currentTime;
}

void
ProfilerEntry::end(double currentTime, uint64 cycleCount)
{
  // std::cout << "SumFrameCounts: " << sumFrameCounts << std::endl;
  
  // Cycles
  lastDeltaCount = cycleCount - startCount;
  sumFrameCounts += lastDeltaCount;

  // Time
  lastDeltaTime = currentTime - startTime;
  sumFrameTime += lastDeltaTime;
  
  numbOfFrameCalls++;
}

void
ProfilerEntry::endFrame()
{
  avgFrameCycleCounts = sumFrameCounts / (float)numbOfFrameCalls;
  avgFrameTime = sumFrameTime / (float)numbOfFrameCalls;
  
  lastNumbOfFrameCalls = numbOfFrameCalls;

  sumGlobalCycleCounts += avgFrameCycleCounts;
  sumGlobalTime += avgFrameTime;
  
  numbOfFrameCalls = 0;
  
  sumFrameCounts = 0;
  sumFrameTime = 0;
}

void
ProfilerBase::start(std::string region)
{
  profilerEntries[region].start(getCurrentTime(), getCurrentCycleCount());
}

void
ProfilerBase::end(std::string region)
{
  profilerEntries[region].end(getCurrentTime(), getCurrentCycleCount());
}

void
ProfilerBase::showData() const
{
  uint64 totalTime = profilerEntries.at("Game").lastDeltaCount;
  
  std::cout << std::endl;
  for(auto i = profilerEntries.begin(); i != profilerEntries.end(); i++)
  {
    const ProfilerEntry& profilerEntry = i->second;
    const std::string& entryName = i->first;
    
    float percentageUse = (float)profilerEntry.lastDeltaCount / totalTime;
    std::cout << std::left << std::setw(16) << entryName;
    
    // Number of iterations
    std:: cout << "It:" << std::setw(10) << profilerEntry.lastNumbOfFrameCalls;
    std::cout << std::setw(12) << "AvgGlCount: " << std::setw(10) << uint64(profilerEntry.sumGlobalCycleCounts / framesElapsed);
    
    if(profilerEntry.lastNumbOfFrameCalls == 1)
    {
      std::cout << std::setw(12) << "CycleCount: " << std::setw(10) << profilerEntry.lastDeltaCount;
      std::cout << std::setw(11)<< "deltaTime: " << std::setw(10) << profilerEntry.lastDeltaTime * 1000000;
    }
    else
    {
      std::cout << std::setw(12) << "AvgCount: " << std::setw(10) << profilerEntry.avgFrameCycleCounts;
      std::cout << std::setw(11) << "sumTime: " << std::setw(10) << profilerEntry.avgFrameTime * 1000000.0f;
      
      float percentageUsePerIteration = (profilerEntry.avgFrameCycleCounts * profilerEntry.lastNumbOfFrameCalls) / totalTime;
      std::cout << "PercSum: " << std::setw(10) << percentageUsePerIteration * 100;
    }
    
    std:: cout << " PercLast: "  << std::setw(10) << percentageUse * 100 << " \t\n";
  }
}

void
ProfilerBase::endFrame()
{
  profilerEntries["Game"].end(getCurrentTime(), getCurrentCycleCount());
  
  for(auto i = profilerEntries.begin(); i != profilerEntries.end(); i++)
  {
    ProfilerEntry& profilerEntry = i->second;
    if(profilerEntry.numbOfFrameCalls > 0)
    {
      profilerEntry.endFrame();
    }
  }
  ++framesElapsed;
}


#if defined(_WIN64) || defined(_WIN32)

Profiler::Profiler()
{
  QueryPerformanceFrequency((LARGE_INTEGER*) &counterFrequency);
}

void
Profiler::startFrame()
{
  start("Game");
}

double
Profiler::getCurrentTime() const
{
  int64 performanceCounter;
  QueryPerformanceCounter((LARGE_INTEGER*)&performanceCounter);
  return performanceCounter / (double) counterFrequency;
}

uint64
Profiler::getCurrentCycleCount() const
{
  uint64 cycleCount = __rdtsc();
  return cycleCount;
}

#endif
