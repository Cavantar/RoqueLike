#pragma once

#include <unordered_map>
#include <string>
#include <assert.h>

#include "jpb.h"
#include "Types.h"

template <typename T>
class Singleton {
public:
  static void create()
  {
    assert(!instance);
    instance = new T;
  }

  static T* get()
  {
    assert(instance);
    return instance;
  }

  static void destroy()
  {
    delete instance;
    instance = NULL;
  }
private:
  static T* instance;
};

template <typename T>
T* Singleton<T>::instance = NULL;

class ProfilerEntry {
public:

  // Cycle Counts ----------------------------
  // Instruction count at the beginning
  uint64 startCount;
  uint64 lastDeltaCount;

  // Total number Of Cycles in frame
  uint64 sumFrameCounts = 0;

  float avgFrameCycleCounts;
  float sumGlobalCycleCounts = 0;

  // Time -----------------------------------

  // Time at the beginning
  double startTime;
  float lastDeltaTime;

  float sumFrameTime = 0;

  float avgFrameTime;
  float sumGlobalTime = 0;

  // Time is in microseconds
  int32 numbOfFrameCalls = 0;
  int32 lastNumbOfFrameCalls = 0;

  void start(double currentTime, uint64 cycleCount);
  void end(double currentTime, uint64 cycleCount);

  void endFrame();
};

class DllExport ProfilerBase {
public:
  virtual void startFrame() = 0;

  void start(std::string region);
  void end(std::string region);

  void showData() const;
  void endFrame();

  virtual double getCurrentTime() const = 0;
  virtual uint64 getCurrentCycleCount() const = 0;
private:
  std::unordered_map<std::string, ProfilerEntry> profilerEntries;
  uint64 framesElapsed = 0;
};

#if defined(_WIN64) || defined(_WIN32)

class DllExport Profiler : public ProfilerBase, public Singleton<Profiler> {
public:
  Profiler();

  void startFrame();

  double getCurrentTime() const;
  uint64 getCurrentCycleCount() const;

private:
  int64 counterFrequency;
};

#endif
