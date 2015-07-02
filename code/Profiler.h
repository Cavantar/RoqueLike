
#pragma once

#include <SFML/Graphics.hpp>

#include <unordered_map>
#include <iostream>
#include <string>

template <typename T>
class Singleton {
public:
  static T* get()
  {
    if(!instance) instance = new T;
    return instance;
  }
private:
  static T* instance;
};

template <typename T>
T* Singleton<T>::instance;

class ProfilerEntry {
public:
  float startTime;
  
  float lastTime;
  float sumTimeFrame;
  
  float avgTimeFrame;
  float avgTimeGlobal;

  int numbOfFrameCalls;
  
  void start(float currentTime);
  void end(float currentTime);

  void endFrame();
};
  
class Profiler {
public:
  virtual void startFrame() = 0;
  
  void start(std::string region);
  void end(std::string region);
  
  void showData() const;
  void endFrame();

  virtual float getCurrentTime() = 0;
private:
  std::unordered_map<std::string, ProfilerEntry> profilerEntries;
};

class SfmlProfiler : public Profiler, public Singleton<SfmlProfiler> {
public:
  void startFrame();
  float getCurrentTime();
private:
  sf::Clock clock;
};
