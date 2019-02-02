#pragma once
#include "basics.h"
#include "Evolver.h"

class View 
{
public:
  View(int width, int height);
  int width;
  int height;
  class Screen* screen; 
  void recordToScreen(Screen* screen);
  void update();
  void setMidC(double x, double y);
  void setC(double x, double y);
  void resetFromHead();
  void load();
  void save();
  
  Evolver* evolvers[4];
};